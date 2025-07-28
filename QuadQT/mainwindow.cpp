#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "loginpage.h"
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    LoginPage *loginPage = new LoginPage(this);
    setCentralWidget(loginPage);
    
    // 로그인 성공 시그널 연결
    connect(loginPage, &LoginPage::loginSuccessful, this, &MainWindow::onLoginSuccessful);
    
    // LoginPage 크기에 맞춰 MainWindow 크기 조정
    setFixedSize(800, 600);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onLoginSuccessful()
{
    // 로그인 성공 후 메인 애플리케이션 화면으로 전환
    QLabel *welcomeLabel = new QLabel("로그인 성공! 메인 애플리케이션 화면입니다.", this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("font-size: 18px; color: #333333;");
    setCentralWidget(welcomeLabel);
}