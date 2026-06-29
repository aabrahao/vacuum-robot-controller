#include "MainWindow.h"
#include "AbstractVideo.h"
#include "CameraVideo.h"
#include "MediaVideo.h"
#include "Settings.h"
#include "WorkspaceView.h"

#include <QAction>
#include <QCameraDevice>
#include <QCloseEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_status(new QLabel(this))
{
    setWindowTitle(tr("Vacuum Robot Controller"));

    Settings &settings = Settings::instance();
    settings.load();
    settings.apply(this);
    settings.center(this);

    setCentralWidget(new QLabel(tr("Loading..."), this));

    createActions();
    createMenus();

    statusBar()->addPermanentWidget(m_status, 1);

    updateActions();
    updateStatus(tr("Ready."));
}

MainWindow::~MainWindow()
{
    if (m_workspace) {
        m_workspace->detachAll();
    }
    qDeleteAll(m_sources);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Settings::instance().capture(this);
    QMainWindow::closeEvent(event);
}

void MainWindow::initialize()
{
    if (m_initialized) {
        return;
    }
    m_initialized = true;

    m_workspace = new WorkspaceView(this);
    setCentralWidget(m_workspace);
    m_workspace->setup();

    connect(m_workspace, &WorkspaceView::statusMessage,
            this, &MainWindow::onStatusMessage);
    connect(m_workspace, &WorkspaceView::errorMessage,
            this, &MainWindow::onErrorMessage);
    connect(m_workspace, &WorkspaceView::sourcesChanged,
            this, &MainWindow::onSourcesChanged);
    connect(m_workspace, &WorkspaceView::controlTriggered,
            this, &MainWindow::onControlTriggered);

    const QCameraDevice camera = CameraVideo::preferred();
    if (!camera.isNull()) {
        addSource(new CameraVideo(camera, this));
    }

    updateActions();
}

WorkspaceView *MainWindow::workspace() const
{
    return m_workspace;
}

QList<AbstractVideo *> MainWindow::sources() const
{
    return m_sources;
}

bool MainWindow::addSource(AbstractVideo *source)
{
    if (!source || !m_workspace) {
        delete source;
        return false;
    }

    const QString key = source->key();
    for (const AbstractVideo *existing : m_sources) {
        if (existing->key() == key) {
            delete source;
            QMessageBox::information(this, tr("Video"),
                                     tr("That source is already playing."));
            return false;
        }
    }

    source->setParent(this);
    m_sources.append(source);

    if (!m_workspace->attach(source)) {
        m_sources.removeOne(source);
        delete source;
        return false;
    }

    updateActions();
    updateStatus();
    return true;
}

void MainWindow::createActions()
{
    m_addCameraAction = new QAction(tr("Add &Camera..."), this);
    m_addCameraAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C));
    connect(m_addCameraAction, &QAction::triggered, this, &MainWindow::addCamera);

    m_openFileAction = new QAction(tr("Open &File..."), this);
    m_openFileAction->setShortcut(QKeySequence::Open);
    connect(m_openFileAction, &QAction::triggered, this, &MainWindow::openFile);

    m_openStreamAction = new QAction(tr("Open &Stream..."), this);
    m_openStreamAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
    connect(m_openStreamAction, &QAction::triggered, this, &MainWindow::openStream);

    m_removeAction = new QAction(tr("&Remove Source..."), this);
    connect(m_removeAction, &QAction::triggered, this, &MainWindow::removeSource);

    m_stopAllAction = new QAction(tr("Sto&p All"), this);
    m_stopAllAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    connect(m_stopAllAction, &QAction::triggered, this, &MainWindow::stopAll);

    m_quitAction = new QAction(tr("&Quit"), this);
    m_quitAction->setShortcut(QKeySequence::Quit);
    connect(m_quitAction, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::createMenus()
{
    QMenu *videoMenu = menuBar()->addMenu(tr("&Video"));
    videoMenu->addAction(m_addCameraAction);
    videoMenu->addAction(m_openFileAction);
    videoMenu->addAction(m_openStreamAction);
    videoMenu->addSeparator();
    videoMenu->addAction(m_removeAction);
    videoMenu->addAction(m_stopAllAction);
    videoMenu->addSeparator();
    videoMenu->addAction(m_quitAction);
}

void MainWindow::addCamera()
{
    const QList<QCameraDevice> devices = CameraVideo::enumerate();
    if (devices.isEmpty()) {
        QMessageBox::warning(this, tr("Add Camera"),
                             tr("No cameras were found."));
        return;
    }

    QStringList names;
    for (const QCameraDevice &device : devices) {
        names.append(device.description());
    }

    bool ok = false;
    const QString choice = QInputDialog::getItem(this, tr("Add Camera"),
                                                 tr("Camera:"), names, 0, false, &ok);
    if (!ok) {
        return;
    }

    const int index = names.indexOf(choice);
    if (index < 0) {
        return;
    }

    addSource(new CameraVideo(devices.at(index), this));
}

void MainWindow::openFile()
{
    const QUrl url = QFileDialog::getOpenFileUrl(
        this,
        tr("Open Video"),
        QString(),
        tr("Video Files (*.mp4 *.mkv *.avi *.mov *.webm);;All Files (*)"));

    if (url.isEmpty()) {
        return;
    }

    addSource(new MediaVideo(url, this));
}

void MainWindow::openStream()
{
    bool ok = false;
    const QString text = QInputDialog::getText(
        this,
        tr("Open Stream"),
        tr("Stream URL:"),
        QLineEdit::Normal,
        QStringLiteral("http://"),
        &ok);

    if (!ok || text.trimmed().isEmpty()) {
        return;
    }

    const QUrl url = QUrl::fromUserInput(text.trimmed());
    if (!url.isValid()) {
        QMessageBox::warning(this, tr("Open Stream"),
                             tr("Invalid URL."));
        return;
    }

    addSource(new MediaVideo(url, this));
}

void MainWindow::removeSource()
{
    if (m_sources.isEmpty()) {
        return;
    }

    QStringList names;
    for (AbstractVideo *source : m_sources) {
        names.append(source->description());
    }

    bool ok = false;
    const QString choice = QInputDialog::getItem(this, tr("Remove Source"),
                                                 tr("Source:"), names, 0, false, &ok);
    if (!ok) {
        return;
    }

    const int index = names.indexOf(choice);
    if (index < 0) {
        return;
    }

    AbstractVideo *source = m_sources.at(index);
    m_workspace->detach(source);
    m_sources.removeOne(source);
    delete source;

    updateActions();
    updateStatus();
}

void MainWindow::stopAll()
{
    if (!m_workspace) {
        return;
    }

    m_workspace->detachAll();

    qDeleteAll(m_sources);
    m_sources.clear();

    updateActions();
    updateStatus(tr("All videos stopped."));
}

void MainWindow::onControlTriggered(const QString &name)
{
    if (name == QStringLiteral("stop")) {
        stopAll();
        return;
    }

    updateStatus(tr("Control: %1").arg(name));
}

void MainWindow::onStatusMessage(const QString &message)
{
    updateStatus(message);
}

void MainWindow::onErrorMessage(const QString &message)
{
    updateStatus(message);
}

void MainWindow::onSourcesChanged()
{
    updateActions();
}

void MainWindow::updateActions()
{
    const bool hasSources = !m_sources.isEmpty();

    m_removeAction->setEnabled(hasSources);
    m_stopAllAction->setEnabled(hasSources);
}

void MainWindow::updateStatus(const QString &message)
{
    if (!message.isEmpty()) {
        m_status->setText(message);
        statusBar()->showMessage(message, 5000);
        return;
    }

    if (!m_workspace) {
        return;
    }

    const QString status = tr("%1 video(s) playing.").arg(m_sources.size());
    m_status->setText(status);
    statusBar()->showMessage(status, 5000);
}
