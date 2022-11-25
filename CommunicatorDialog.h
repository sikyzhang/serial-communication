//
// Created by qwe22 on 2022-03-20.
//

#ifndef SERIALCOMMUNICATION_COMMUNICATOR_DIALOG_H
#define SERIALCOMMUNICATION_COMMUNICATOR_DIALOG_H

#include <bits/stdc++.h>

#include <QApplication>
#include <QString>
#include <QPushButton>
#include <QLabel>
#include <QDialog>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QImageReader>
#include <QMessageBox>
#include <Qimage>
#include <QScrollArea>
#include <QComboBox>
#include <QTextCodec>
#include <QTextEdit>

#include "CSerialPort/SerialPort.h"
#include "CSerialPort/SerialPortInfo.h"

#ifdef _WIN32

#include <windows.h>

#define imsleep(microsecond) Sleep(microsecond) // ms
#else
#include <unistd.h>
#define imsleep(microsecond) usleep(1000 * microsecond) // ms
#endif

const char HINT_NO_FILE[] = "找不到文件，请先选择文件";
const char HINT_FIND_FILE[] = "找到文件：";
const char HINT_NEED_LOADING[] = "点击右上方按钮加载图片";

class CommunicatorDialog : public QDialog, public has_slots<> {
Q_OBJECT
public:
    CommunicatorDialog();

public slots:

    void changePort(int idx);       // 设置发送使用的串口
    void refreshSerials();             // 刷新接收串口列表
    void openSerialButton_clicked();   // 打开串口
    void changeCommand(int idx);       // 选择信息
    void sendMessage();                // 发送编辑框里的信息
    void receiveMessage();             // 读取对应串口接收的信息
private:
    QVBoxLayout *body;

    QPushButton *refreshSerialsButton; // 刷新串口信息
    QPushButton *openSerialButton;     // 打开串口按钮
    QComboBox *serialSelector;         // 发送串口选择器
    QPushButton *sendButton;           // 发送串口信息
    QComboBox *commandSelector;        // 串口命令选择器
    QLineEdit *messageForwardEditor;   // 串口消息编辑器
    QTextEdit *messageSentShower;      // 串口消息展示器（发送）
    QTextEdit *messageReceivedShower;  // 串口消息展示器（接收）

    vector<SerialPortInfo> serialPortInfos; // 记录串口列表
    static const int BUFFER_SIZE = 4096;
    static const int BAUD_RATE = 9600;
    static const itas109::Parity PARITY = itas109::ParityNone;
    static const itas109::DataBits DATA_BITS = itas109::DataBits8;
    static const itas109::StopBits STOP_BITS = itas109::StopOne;
    static const itas109::FlowControl FLOW_CONTROL = itas109::FlowNone;
    int curSerialIdx{};
    itas109::CSerialPort curSerialPort;
    map<string, string> commandsMap;

    void refreshSerialList(); // 刷新串口列表
    void refreshCommands();   // 刷新命令列表
    bool sendMessage(const string &msg); // 发送二进制数据
    bool openSerial(); // 打开串口

    static string toBytes(const QString &hex); // 将 HEX 格式的数据转换为二进制
    static string beautifyHex(const string &hex); // 在16进制的字符串中插入空格
    static map<string, string> readCommands(const string &commandsFilePath = "config/commands.txt"); // 从txt文件中读取命令列表
    static vector<string> readFile(const string &filePath); // 从文件中读取行（删去注释）
    static vector<string> split(const string& str, char point = ' ');  // 按照分割字符分割字符串
    static string timestamp(); // 获取当前时刻时间戳
    static string toHEX(const string &s);                    // 把字符串变成16进制字符串
    static string toHEX(const char *s, size_t len);          // 把字符数组变成16进制字符串
    static string toHEX(const unsigned char *s, size_t len); // 把字符数组变成16进制字符串
};

#endif //SERIALCOMMUNICATION_COMMUNICATOR_DIALOG_H
