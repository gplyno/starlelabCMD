#include "terminal.h"

#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QShortcut>
#include <QFont>
#include <QScrollBar>
#include <QTextCursor>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QTimer>
#include <QUrl>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfoList>

Terminal::Terminal(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setWindowTitle("Starlelab CMD");
    resize(720, 480);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_titleBar = new QWidget(this);
    m_titleBar->setFixedHeight(30);

    auto *tbLayout = new QHBoxLayout(m_titleBar);
    tbLayout->setContentsMargins(12, 0, 0, 0);
    tbLayout->setSpacing(0);

    m_titleLabel = new QLabel("Starlelab CMD", m_titleBar);

    m_closeBtn = new QPushButton("\u2715", m_titleBar);
    m_closeBtn->setFixedSize(46, 30);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    m_closeBtn->setStyleSheet(
        "QPushButton { color:#e0e0e0; background:transparent; border:none;"
        "              font-size: 11pt; }"
        "QPushButton:hover { background:#c42b1c; color:white; }");
    connect(m_closeBtn, &QPushButton::clicked, qApp, &QApplication::quit);

    tbLayout->addWidget(m_titleLabel);
    tbLayout->addStretch();
    tbLayout->addWidget(m_closeBtn);

    m_output = new QTextEdit(this);
    m_output->setReadOnly(true);
    m_output->setFrameStyle(0);
    m_output->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_output->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QFont mono("Consolas");
    mono.setStyleHint(QFont::Monospace);
    mono.setPointSize(11);
    m_output->setFont(mono);
    m_output->setLineWrapMode(QTextEdit::WidgetWidth);

    m_inputRow = new QWidget(this);

    auto *inputLayout = new QHBoxLayout(m_inputRow);
    inputLayout->setContentsMargins(12, 4, 12, 4);
    inputLayout->setSpacing(8);

    m_prompt = new QLabel(">", m_inputRow);
    m_prompt->setFont(mono);
    m_prompt->setFixedWidth(12);

    m_input = new QLineEdit(m_inputRow);
    m_input->setPlaceholderText("Type a command and press Enter...");
    m_input->setFont(mono);
    m_input->setCursor(Qt::IBeamCursor);
    m_input->installEventFilter(this);
    m_input->setFocusPolicy(Qt::StrongFocus);

    inputLayout->addWidget(m_prompt);
    inputLayout->addWidget(m_input, 1);

    m_hint = new QLabel(
        "\u2191/\u2193 \u2014 history   \u2022   Tab \u2014 autocomplete   "
        "\u2022   Ctrl+L \u2014 clear   \u2022   F1 \u2014 hints   "
        "\u2022   Enter \u2014 run",
        this);
    m_hint->setFont(mono);

    root->addWidget(m_titleBar);
    root->addWidget(m_output, 1);
    root->addWidget(m_inputRow);
    root->addWidget(m_hint);

    // Global shortcuts: work regardless of focus.
    auto *scHints = new QShortcut(QKeySequence(Qt::Key_F1), this);
    connect(scHints, &QShortcut::activated, this, &Terminal::toggleHints);

    auto *scClear = new QShortcut(QKeySequence("Ctrl+L"), this);
    connect(scClear, &QShortcut::activated, this, [this] { m_output->clear(); });

    connect(m_input, &QLineEdit::returnPressed,
            this, &Terminal::onCommandEntered);

    applyTheme();
    printBanner();

    QTimer::singleShot(0, m_input, [this] { m_input->setFocus(); });
}

void Terminal::applyTheme() {
    if (m_darkTheme) {
        m_colText    = "#c8c8c8";
        m_colMuted   = "#7a7a7a";
        m_colError   = "#e06c75";
        m_colSuccess = "#98c379";
        m_colAccent  = "#61afef";
        m_colDivider = "#2a2a2a";
    } else {
        m_colText    = "#1a1a1a";
        m_colMuted   = "#888888";
        m_colError   = "#c0392b";
        m_colSuccess = "#27ae60";
        m_colAccent  = "#2980b9";
        m_colDivider = "#d0d0d0";
    }

    const QString bg     = m_darkTheme ? "#0b0b0b" : "#f5f5f5";
    const QString inputB = m_darkTheme ? "#141414" : "#ffffff";
    const QString inputF = m_darkTheme ? "#181818" : "#f0f0f0";
    const QString border = m_darkTheme ? "#2a2a2a" : "#d0d0d0";

    setStyleSheet(QString("Terminal { background-color: %1; }").arg(bg));

    m_titleBar->setStyleSheet(QString("background-color: %1;").arg(bg));
    m_titleLabel->setStyleSheet(QString(
                                    "color:%1; font-family: Consolas; font-size: 10pt;").arg(m_colText));
    m_closeBtn->setStyleSheet(
        "QPushButton { color:#e0e0e0; background:transparent; border:none;"
        "              font-size: 11pt; }"
        "QPushButton:hover { background:#c42b1c; color:white; }");

    m_output->setStyleSheet(QString(
                                "QTextEdit { background-color: %1; color: %2;"
                                "            border: none; padding: 10px 12px 6px 12px; }"
                                "QScrollBar:vertical, QScrollBar:horizontal"
                                "            { background: transparent; width: 0; height: 0; margin: 0; }"
                                "QScrollBar::handle:vertical, QScrollBar::handle:horizontal"
                                "            { background: transparent; }"
                                "QScrollBar::add-line, QScrollBar::sub-line"
                                "            { height: 0; width: 0; }")
                                .arg(bg, m_colText));

    m_inputRow->setStyleSheet(QString("background:%1;").arg(bg));
    m_prompt->setStyleSheet(QString("color:%1; font-size:11pt;").arg(m_colText));

    m_input->setStyleSheet(QString(
                               "QLineEdit { background-color: %1; color: %2;"
                               "            border: 1px solid %3; border-radius: 4px;"
                               "            padding: 8px 12px;"
                               "            selection-background-color: #3a6ea5; }"
                               "QLineEdit:focus { border: 1px solid #4a90d9;"
                               "                  background-color: %4; }")
                               .arg(inputB, m_colText, border, inputF));

    m_hint->setStyleSheet(QString(
                              "QLabel { color: %1; background: %2;"
                              "         padding: 0 12px 8px 12px; font-size: 9pt; }")
                              .arg(m_colMuted, bg));
}

void Terminal::scrollToBottom() {
    auto *sb = m_output->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void Terminal::appendOutput(const QString &text, const QString &color) {
    const QString c = color.isEmpty() ? m_colText : color;
    m_output->moveCursor(QTextCursor::End);
    m_output->insertHtml(QString(
                             "<span style=\"color:%1; white-space:pre-wrap;\">%2</span>")
                             .arg(c,
                                  text.toHtmlEscaped().replace("\n", "<br>").replace(" ", "&nbsp;")));
    scrollToBottom();
}

void Terminal::printBanner() {
    appendOutput("Welcome to starlelab cmd\n");
    appendOutput("Type 'help' to see available commands.\n", m_colMuted);
    appendOutput(QString(60, QChar(0x2500)) + "\n", m_colDivider);
}

void Terminal::onCommandEntered() {
    const QString cmd = m_input->text();
    m_input->clear();

    appendOutput("> " + cmd + "\n");

    if (m_awaitingSaveName) {
        m_awaitingSaveName = false;
        const QString name = cmd.trimmed();
        if (!name.isEmpty()) cmdDeleteSave(name);
        return;
    }

    const QString trimmed = cmd.trimmed();
    if (trimmed.isEmpty()) return;

    if (m_history.isEmpty() || m_history.last() != trimmed)
        m_history << trimmed;
    m_historyIndex = m_history.size();

    processCommand(trimmed);
}

void Terminal::processCommand(const QString &cmd) {
    const QString lower = cmd.toLower();

    if (lower == "help")                    { cmdHelp();         return; }
    if (lower == "about")                   { cmdAbout();        return; }
    if (lower == "gi")                      { cmdGameInfo();     return; }
    if (lower == "dlg")                     { cmdDownloadGame(); return; }
    if (lower == "ws")                      { cmdWebsite();      return; }
    if (lower == "cs")                      { cmdListSaves();    return; }
    if (lower == "ds") {
        appendOutput("Enter save name to delete: ");
        m_awaitingSaveName = true;
        return;
    }
    if (lower.startsWith("ds "))            { cmdDeleteSave(cmd.mid(3).trimmed()); return; }
    if (lower == "blog")                    { cmdBlog();         return; }
    if (lower == "thoughts")                { cmdThoughts();     return; }
    if (lower == "cv")                      { cmdCheckVersion(); return; }
    if (lower == "color")                   { cmdColor();        return; }
    if (lower == "hints")                   { toggleHints();     return; }
    if (lower == "clear" || lower == "cls") { m_output->clear(); return; }
    if (lower == "exit" || lower == "quit") { qApp->quit();      return; }

    appendOutput("Unknown command: " + cmd + "\n", m_colError);
    appendOutput("Type 'help' for a list of commands.\n", m_colMuted);
}

void Terminal::cmdHelp() {
    appendOutput(
        "Commands:\n"
        "  about     - info about starlelab cmd\n"
        "  help      - this list\n"
        "  gi        - game info\n"
        "  dlg       - download game\n"
        "  ws        - website\n"
        "  cs        - list saves\n"
        "  ds        - delete a save (asks for name)\n"
        "  blog      - open blog page in browser\n"
        "  thoughts  - open thoughts page in browser\n"
        "  cv        - check current version (opens pages)\n"
        "  color     - toggle dark / light theme\n"
        "  hints     - show/hide bottom hint bar\n"
        "  exit      - quit\n");
}

void Terminal::cmdAbout() {
    appendOutput("starlelab cmd: developer console and game download\n");
}

void Terminal::cmdGameInfo() {
    appendOutput(
        "Zombotanic is a survival RPG set in a world where nature has become "
        "corrupted. Explore a dangerous world, gather resources, build your "
        "shelter, and fight against the growing zombie infection. Combine "
        "different playstyles with RPG progression, roguelike elements, and "
        "survival mechanics. Every journey is different, and every decision "
        "can determine how long you survive. The game is currently in early "
        "development, but there is much more to come. We are actively "
        "improving Zombotanic and listening to player feedback.\n");
}

void Terminal::cmdDownloadGame() {
    appendOutput("Opening download page...\n", m_colMuted);
    QDesktopServices::openUrl(QUrl(
        "https://starlelab.itch.io/zombotanic/download/"
        "soVqWcfAj6Okf6FXjFZp3COIlv5GN3OXMmTXvYt0"));
}

void Terminal::cmdWebsite() {
    appendOutput("Opening website...\n", m_colMuted);
    QDesktopServices::openUrl(QUrl("https://gplyno.github.io/starlelab/"));
}

QString Terminal::savesPath() const {
    // Windows stores saves under %LOCALAPPDATA%/Zombotanic/saves.
#ifdef Q_OS_WIN
    return QDir::cleanPath(
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
        "/Zombotanic/saves");
#else
    return QDir::cleanPath(
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) +
        "/Zombotanic/saves");
#endif
}

void Terminal::cmdListSaves() {
    QDir dir(savesPath());

    if (!dir.exists()) {
        appendOutput("Saves folder not found: " + savesPath() + "\n", m_colError);
        return;
    }

    const QFileInfoList entries = dir.entryInfoList(
        QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    if (entries.isEmpty()) {
        appendOutput("No saves found.\n", m_colMuted);
        return;
    }

    appendOutput("Saves (" + QString::number(entries.size()) + "):\n");
    for (const QFileInfo &fi : entries)
        appendOutput("  " + fi.fileName() + "\n");
}

void Terminal::cmdDeleteSave(const QString &name) {
    if (name.isEmpty()) {
        appendOutput("Usage: ds <save name>\n", m_colError);
        return;
    }

    QDir dir(savesPath() + "/" + name);
    if (!dir.exists()) {
        appendOutput("Save not found: " + name + "\n", m_colError);
        return;
    }

    if (dir.removeRecursively())
        appendOutput("Deleted save: " + name + "\n", m_colSuccess);
    else
        appendOutput("Failed to delete save: " + name + "\n", m_colError);
}

void Terminal::cmdBlog() {
    appendOutput("Opening blog in browser...\n", m_colMuted);
    QDesktopServices::openUrl(
        QUrl("https://gplyno.github.io/starlelab/?page=blog"));
}

void Terminal::cmdThoughts() {
    appendOutput("Opening thoughts in browser...\n", m_colMuted);
    QDesktopServices::openUrl(
        QUrl("https://gplyno.github.io/starlelab/?page=thoughts"));
}

void Terminal::cmdCheckVersion() {
    appendOutput("Opening version pages...\n", m_colMuted);
    appendOutput("  itch.io: https://starlelab.itch.io/zombotanic\n", m_colMuted);
    appendOutput("  history: https://gplyno.github.io/starlelab/?page=history\n",
                 m_colMuted);
    QDesktopServices::openUrl(QUrl("https://starlelab.itch.io/zombotanic"));
    QDesktopServices::openUrl(
        QUrl("https://gplyno.github.io/starlelab/?page=history"));
}

void Terminal::cmdColor() {
    m_darkTheme = !m_darkTheme;
    applyTheme();
    // Existing log lines carry colors baked into HTML, so clear them
    // and reprint the banner with the new palette.
    m_output->clear();
    printBanner();
    appendOutput(QString("Theme: %1\n")
                     .arg(m_darkTheme ? "dark" : "light"),
                 m_colAccent);
}

void Terminal::toggleHints() {
    if (!m_hint) return;
    m_hint->isVisible() ? m_hint->hide() : m_hint->show();
    if (m_input) m_input->setFocus();
}

void Terminal::navigateHistory(int direction) {
    if (m_history.isEmpty()) return;

    m_historyIndex += direction;
    if (m_historyIndex < 0) m_historyIndex = 0;
    if (m_historyIndex >= m_history.size()) m_historyIndex = m_history.size();

    if (m_historyIndex == m_history.size())
        m_input->clear();
    else
        m_input->setText(m_history.at(m_historyIndex));
}

void Terminal::autocomplete() {
    const QString text = m_input->text();
    if (text.isEmpty()) return;

    QStringList matches;
    for (const QString &c : m_commands)
        if (c.startsWith(text, Qt::CaseInsensitive)) matches << c;

    if (matches.size() == 1)
        m_input->setText(matches.first());
    else if (matches.size() > 1) {
        appendOutput("> " + text + "\n");
        appendOutput(matches.join("   ") + "\n", m_colMuted);
    }
}

// Input-only hotkeys; F1 and Ctrl+L are on QShortcut in the ctor.
bool Terminal::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_input && event->type() == QEvent::KeyPress) {
        auto *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Up)   { navigateHistory(-1); return true; }
        if (ke->key() == Qt::Key_Down) { navigateHistory(1);  return true; }
        if (ke->key() == Qt::Key_Tab)  { autocomplete();      return true; }
    }
    return QWidget::eventFilter(obj, event);
}

void Terminal::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        if (m_titleBar && m_titleBar->geometry().contains(e->pos())) {
            m_dragging = true;
            m_dragPos  = e->globalPosition().toPoint() - frameGeometry().topLeft();
            e->accept();
            return;
        }
        if (m_input) m_input->setFocus();
    }
    QWidget::mousePressEvent(e);
}

void Terminal::mouseMoveEvent(QMouseEvent *e) {
    if (m_dragging && (e->buttons() & Qt::LeftButton)) {
        move(e->globalPosition().toPoint() - m_dragPos);
        e->accept();
    }
}

void Terminal::mouseReleaseEvent(QMouseEvent *e) {
    Q_UNUSED(e);
    m_dragging = false;
}