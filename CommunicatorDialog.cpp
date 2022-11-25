#include "CommunicatorDialog.h"

CommunicatorDialog::CommunicatorDialog() {
    // 窗口基础设置
    this->setWindowTitle("串口通讯");

    // 设置窗口主体垂直布局器
    body = new QVBoxLayout(this);
    // 设置水平布局器
    QHBoxLayout *hBox{};

    // 添加一行，串口选择打开控件
    hBox = new QHBoxLayout();
    serialSelector = new QComboBox(); // 发送串口选择器
    refreshSerialsButton = new QPushButton("刷新"); // 刷新串口信息按钮
    openSerialButton = new QPushButton("打开串口");
    hBox->addWidget(serialSelector);
    hBox->addWidget(refreshSerialsButton);
    hBox->addWidget(openSerialButton);
    body->addLayout(hBox);

    // 添加一行，命令选择控件
    hBox = new QHBoxLayout();
    hBox->addWidget(new QLabel(tr("选择命令：")));
    commandSelector = new QComboBox();
    hBox->addWidget(commandSelector, 3);
    body->addLayout(hBox);

    // 添加一行，串口消息编辑控件
    hBox = new QHBoxLayout();
    messageForwardEditor = new QLineEdit(); // 串口信息编辑文本框
    sendButton = new QPushButton("发送"); // 发送串口信息按钮
    hBox->addWidget(messageForwardEditor, 3);
    hBox->addWidget(sendButton);
    body->addLayout(hBox);

    // 添加一行，串口发送消息展示器标签
    hBox = new QHBoxLayout();
    hBox->addWidget(new QLabel(tr("已发送消息：")));
    body->addLayout(hBox);

    // 添加一行，串口发送消息展示器标签
    hBox = new QHBoxLayout();
    messageSentShower = new QTextEdit();
    messageSentShower->setReadOnly(true);
    hBox->addWidget(messageSentShower);
    body->addLayout(hBox);

    // 添加一行，串口接收消息展示器标签
    hBox = new QHBoxLayout();
    hBox->addWidget(new QLabel(tr("已接收消息：")));
    body->addLayout(hBox);

    // 添加一行，串口接收消息展示器
    hBox = new QHBoxLayout();
    messageReceivedShower = new QTextEdit();
    messageReceivedShower->setReadOnly(true);
    hBox->addWidget(messageReceivedShower);
    body->addLayout(hBox);

    // 初始化整个窗口
    setLayout(body);

    // 设置信号与槽函数的链接
    connect(commandSelector, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changeCommand(int)));
    connect(refreshSerialsButton, SIGNAL(clicked(bool)),
            this, SLOT(refreshSerials()));
    connect(openSerialButton, SIGNAL(clicked(bool)),
            this, SLOT(openSerialButton_clicked()));
    connect(serialSelector, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changePort(int)));
    connect(sendButton, SIGNAL(clicked(bool)),
            this, SLOT(sendMessage()));

    // 连接串口
    curSerialPort.readReady.connect(this, &CommunicatorDialog::receiveMessage);

    // 初始化
    refreshCommands();
    refreshSerialsButton->clicked();
}

void CommunicatorDialog::refreshSerials() {
    this->refreshSerialList();
}

void CommunicatorDialog::refreshSerialList() {
    serialPortInfos = CSerialPortInfo::availablePortInfos();
    QStringList serialDescriptions;
    for (const SerialPortInfo &info: serialPortInfos) {
        QString str = QString::fromLocal8Bit(info.portName.c_str()) + QString(":") +
                      QString::fromLocal8Bit(info.description.c_str());
        serialDescriptions.append(str);
    }
    int outIndex = serialDescriptions.indexOf(serialSelector->currentText());
    if (outIndex < 0) { outIndex = 0; }
    serialSelector->clear();
    serialSelector->insertItems(0, serialDescriptions);
    serialSelector->setCurrentIndex(outIndex);
    serialSelector->setEnabled(true);
    if (serialPortInfos.empty()) {
        serialSelector->insertItem(0, "在系统中未检测到串口");
        serialSelector->setEnabled(false);
    }
}

void CommunicatorDialog::changePort(int idx) {
    this->curSerialIdx = idx;
    if (curSerialPort.isOpened()) { openSerialButton->clicked(); }
}

void CommunicatorDialog::openSerialButton_clicked() {
    if (openSerialButton->text() == tr("打开串口")) {
        if (!openSerial()) {
            QMessageBox::information(
                    this,
                    tr("错误"),
                    tr("串口打开失败！")
            );
        }
        openSerialButton->setText(tr("关闭串口"));
    } else if (openSerialButton->text() == tr("关闭串口")) {
        curSerialPort.close();
        openSerialButton->setText(tr("打开串口"));
    }
}

bool CommunicatorDialog::openSerial() {
    SerialPortInfo &curSerialPortInfo = serialPortInfos[curSerialIdx];
    curSerialPort.init(
            curSerialPortInfo.portName,
            BAUD_RATE,
            PARITY,
            DATA_BITS,
            STOP_BITS,
            FLOW_CONTROL,
            BUFFER_SIZE
    );
    curSerialPort.open();
    if (!curSerialPort.isOpened()) { return false; }
    return true;
}

void CommunicatorDialog::refreshCommands() {
    QStringList itemList;
    commandsMap = readCommands();
    for (auto &command: commandsMap) {
        itemList.append(tr(command.first.c_str()));
    }
    commandSelector->clear();
    commandSelector->addItems(itemList);
}

void CommunicatorDialog::changeCommand(int idx) {
    QString commandName = commandSelector->itemText(idx);
    QString commandMessage = QString::fromStdString(commandsMap[commandName.toStdString()]);
    messageForwardEditor->setText(commandMessage);
}

void CommunicatorDialog::sendMessage() {
    sendMessage(toBytes(messageForwardEditor->text()));
}

bool CommunicatorDialog::sendMessage(const string &msg) {
    SerialPortInfo &curSerialPortInfo = serialPortInfos[curSerialIdx];
    if (!curSerialPort.isOpened()) {
        QMessageBox::information(
                this,
                tr("错误"),
                tr("请先打开串口！")
        );
        return false;
    }
    curSerialPort.writeData(msg.c_str(), (int) msg.size());
    string hex = beautifyHex(toHEX(msg.c_str(), msg.size()));
    string msgLog = timestamp() + ":\n" + hex;
    messageSentShower->insertPlainText(QString::fromStdString(msgLog) + "\n");
    return true;
}

void CommunicatorDialog::receiveMessage() {
    int rec_size;
    char *rec_buf = new char[BUFFER_SIZE];
    rec_size = curSerialPort.readAllData(rec_buf);
    string hex = beautifyHex(toHEX(rec_buf, rec_size));
    string msgLog = timestamp() + ":\n" + hex;
    messageReceivedShower->insertPlainText(QString::fromStdString(msgLog) + "\n");
    delete[] rec_buf;
}

map<string, string> CommunicatorDialog::readCommands(const string &commandsFilePath) {
    map<string, string> res;
    vector<string> lines = readFile(commandsFilePath);
    for (const string &line: lines) {
        vector<string> item = split(line);
        res[item[0]] = item[1];
    }
    return res;
}

string CommunicatorDialog::toBytes(const QString &hex) {
    string s_hex = hex.toStdString(), res;
    for (char &i: s_hex) {
        if ('0' <= i && i <= '9') { i -= '0'; }
        else if ('A' <= i && i <= 'F') { i -= 'A' - 10; }
        else if ('a' <= i && i <= 'f') { i -= 'a' - 10; }
    }
    for (int i = 0; i < s_hex.size(); i += 2) {
        res += (char) (s_hex[i] * 16 + s_hex[i + 1]);
    }
    return res;
}

string CommunicatorDialog::beautifyHex(const string &hex) {
    string res = string(hex.size() + hex.size() / 2, ' ');
    for (int i = 0; i < hex.size(); ++i) {
        res[i + i / 2] = hex[i];
    }
    return res;
}

vector<string> CommunicatorDialog::readFile(const string &filePath) {
    fstream file = fstream(filePath);
    vector<string> res;
    char buf[BUFFER_SIZE];
    while (!file.eof()) {
        int beg = 0;
        file.getline(buf, BUFFER_SIZE);
        while (buf[beg] == ' ') { beg += 1; }
        if (buf[beg] == '#' || buf[beg] == 0) { continue; }
        res.emplace_back(buf);
    }
    return res;
}

vector<string> CommunicatorDialog::split(const string &str, char point) {
    vector<string> res;
    int cur_beg = 0;
    for (int i = 0; i < str.size(); ++i) {
        if (str[i] == point && i - cur_beg > 0) {
            res.push_back(str.substr(cur_beg, i - cur_beg));
            cur_beg = i + 1;
        }
    }
    if (str.size() - cur_beg > 0) { res.push_back(str.substr(cur_beg, str.size() - cur_beg)); }
    return res;
}

string CommunicatorDialog::toHEX(const string &s) {
    return toHEX((unsigned char *) s.c_str(), s.size());
}

string CommunicatorDialog::toHEX(const char *s, size_t len) {
    return toHEX((unsigned char *) s, len);
}

string CommunicatorDialog::toHEX(const unsigned char *s, size_t len) {
    const char HEX[] = {'0', '1', '2', '3', '4',
                        '5', '6', '7', '8', '9',
                        'A', 'B', 'C', 'D', 'E', 'F'};
    char *res = new char[len * 2 + 10]{};
    for (int i = 0; i < len; i++) {
        res[2 * i] = HEX[s[i] / 16];
        res[2 * i + 1] = HEX[s[i] % 16];
    }
    return (string) res;
}

string CommunicatorDialog::timestamp() {
    char res[22];
    auto now = chrono::system_clock::now();
    time_t time = chrono::system_clock::to_time_t(now);
    strftime(res, 20, "%H:%M:%S", localtime(&time));
    return (string) res;
}

