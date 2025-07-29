#include "mainwindow/topbarwidget.h"
#include <QLabel>
#include <QFont>
#include <QFontDatabase>
#include <QDebug>
#include <QMouseEvent>
#include <QPixmap>
#include <QMenu>
#include <QAction>
#include <QFile>
#include <QDir>

TopBarWidget::TopBarWidget(QWidget *parent)
    : QWidget(parent), m_activeButton(TopBarButton::Camera), m_parentWidth(0), m_parentHeight(0)
{
    // QuadZone 로고 - HanwhaB 폰트 적용
    logoLabel = new QLabel("QuadZone", this);
    
    // 실제 로드된 한화 폰트 패밀리 찾기
    QStringList allFamilies = QFontDatabase().families();
    QString hanwhaBFamily;
    
    // HanwhaB 폰트 찾기 (Bold 폰트 우선)
    for (const QString &family : allFamilies) {
        if (family.contains("Hanwha", Qt::CaseInsensitive) && 
            (family.contains("B", Qt::CaseInsensitive) || family.contains("Bold", Qt::CaseInsensitive)) &&
            !family.contains("Gothic", Qt::CaseInsensitive)) {
            hanwhaBFamily = family;
            break;
        }
    }
    
    // HanwhaB가 없으면 일반 Hanwha 폰트 찾기
    if (hanwhaBFamily.isEmpty()) {
        for (const QString &family : allFamilies) {
            if (family.contains("Hanwha", Qt::CaseInsensitive)) {
                hanwhaBFamily = family;
                break;
            }
        }
    }
    
    qDebug() << "TopBarWidget QuadZone 로고용 폰트:" << hanwhaBFamily;
    
    QFont logoFont(hanwhaBFamily.isEmpty() ? "Arial" : hanwhaBFamily, 24);  // 크기 24로 증가
    logoFont.setBold(true);
    logoLabel->setFont(logoFont);
    logoLabel->setStyleSheet("color: #F37321;");
    qDebug() << "TopBar QuadZone 로고 폰트 설정:" << logoFont.family() << "크기:" << logoFont.pointSize();

    // 아이콘들을 이미지로 설정
    cameraIcon = new QLabel(this);
    docIcon = new QLabel(this);
    settingIcon = new QLabel(this);
    loginStatus = new QLabel(this);

    setupIcons();

    topLine = new QWidget(this);
    topLine->setStyleSheet("background-color: #ccc;");
    
    // 기본적으로 카메라 버튼이 활성화된 상태로 시작
    setActiveButton(TopBarButton::Camera);
}

void TopBarWidget::setupIcons()
{
    qDebug() << "아이콘 설정 시작";
    
    // 리소스 시스템이 제대로 작동하는지 확인
    qDebug() << "리소스 경로 테스트 (true = 실패, false = 성공):";
    qDebug() << "  - 카메라:" << QPixmap(":/images/camera.png").isNull();
    qDebug() << "  - 문서:" << QPixmap(":/images/Document.png").isNull();
    qDebug() << "  - 설정:" << QPixmap(":/images/Settings.png").isNull();
    qDebug() << "  - 프로필:" << QPixmap(":/images/Profile.png").isNull();
    
    // 다양한 절대 경로 테스트
    qDebug() << "절대 경로 테스트:";
    QStringList testPaths = {
        "resources/images/camera.png",
        "./resources/images/camera.png", 
        "../resources/images/camera.png",
        "../../resources/images/camera.png"
    };
    
    for (const QString &path : testPaths) {
        bool exists = QFile::exists(path);
        bool loads = !QPixmap(path).isNull();
        qDebug() << "  경로:" << path << "존재:" << exists << "로드:" << loads;
    }
    
    updateButtonStates();
}

void TopBarWidget::setActiveButton(TopBarButton button)
{
    m_activeButton = button;
    updateButtonStates();
}

void TopBarWidget::updateButtonStates()
{
    // 카메라 버튼
    QPixmap cameraPixmap;
    if (m_activeButton == TopBarButton::Camera) {
        cameraPixmap = QPixmap(":/images/camera_orange.png");
        qDebug() << "카메라 오렌지 이미지 로드:" << (!cameraPixmap.isNull() ? "성공" : "실패");
        if (cameraPixmap.isNull()) {
            // Fallback: 절대 경로 시도
            cameraPixmap = QPixmap("../../resources/images/camera_orange.png");
            qDebug() << "카메라 오렌지 절대경로 시도:" << (!cameraPixmap.isNull() ? "성공" : "실패");
        }
    } else {
        cameraPixmap = QPixmap(":/images/camera.png");
        qDebug() << "카메라 일반 이미지 로드:" << (!cameraPixmap.isNull() ? "성공" : "실패");
        if (cameraPixmap.isNull()) {
            cameraPixmap = QPixmap("../../resources/images/camera.png");
            qDebug() << "카메라 일반 절대경로 시도:" << (!cameraPixmap.isNull() ? "성공" : "실패");
        }
    }
    
    // 이미지 로드 실패 시 텍스트 fallback
    if (cameraPixmap.isNull()) {
        cameraIcon->clear();
        cameraIcon->setText("CAM");
        cameraIcon->setStyleSheet(m_activeButton == TopBarButton::Camera ? 
            "background-color: #F37321; color: white; font-weight: bold; border-radius: 8px;" : 
            "background-color: #eee; color: black; font-weight: bold; border-radius: 8px;");
        cameraIcon->setAlignment(Qt::AlignCenter);
    } else {
        // 버튼 크기에 비례한 아이콘 크기 계산 (버튼 크기의 100%)
        double w_unit = m_parentWidth / 24.0;
        double h_unit = m_parentHeight / 24.0;
        int iconPixelSize = qMin(w_unit * 1.4 * 1.0, h_unit * 1.4 * 1.0);
        
        // 이미지 크기를 동적으로 조정하여 비율 유지
        QPixmap scaledPixmap = cameraPixmap.scaled(iconPixelSize, iconPixelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        cameraIcon->setPixmap(scaledPixmap);
        cameraIcon->setScaledContents(false);
        cameraIcon->setAlignment(Qt::AlignCenter);
        cameraIcon->setStyleSheet("border-radius: 8px;");
    }
    
    // 문서 버튼
    QPixmap docPixmap;
    if (m_activeButton == TopBarButton::Document) {
        docPixmap = QPixmap(":/images/Document_orange.png");
        if (docPixmap.isNull()) docPixmap = QPixmap("resources/images/Document_orange.png");
    } else {
        docPixmap = QPixmap(":/images/Document.png");
        if (docPixmap.isNull()) docPixmap = QPixmap("resources/images/Document.png");
    }
    
    if (docPixmap.isNull()) {
        docIcon->clear();
        docIcon->setText("DOC");
        docIcon->setStyleSheet(m_activeButton == TopBarButton::Document ? 
            "background-color: #F37321; color: white; font-weight: bold; border-radius: 8px;" : 
            "background-color: #eee; color: black; font-weight: bold; border-radius: 8px;");
        docIcon->setAlignment(Qt::AlignCenter);
    } else {
        // 버튼 크기에 비례한 아이콘 크기 계산 (버튼 크기의 100%)
        double w_unit = m_parentWidth / 24.0;
        double h_unit = m_parentHeight / 24.0;
        int iconPixelSize = qMin(w_unit * 1.4 * 1.0, h_unit * 1.4 * 1.0);
        
        QPixmap scaledPixmap = docPixmap.scaled(iconPixelSize, iconPixelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        docIcon->setPixmap(scaledPixmap);
        docIcon->setScaledContents(false);
        docIcon->setAlignment(Qt::AlignCenter);
        docIcon->setStyleSheet("border-radius: 8px;");
    }
    
    // 설정 버튼
    QPixmap settingsPixmap;
    if (m_activeButton == TopBarButton::Settings) {
        settingsPixmap = QPixmap(":/images/settings_orange.png");
        if (settingsPixmap.isNull()) settingsPixmap = QPixmap("resources/images/settings_orange.png");
    } else {
        settingsPixmap = QPixmap(":/images/Settings.png");
        if (settingsPixmap.isNull()) settingsPixmap = QPixmap("resources/images/Settings.png");
    }
    
    if (settingsPixmap.isNull()) {
        settingIcon->clear();
        settingIcon->setText("SET");
        settingIcon->setStyleSheet(m_activeButton == TopBarButton::Settings ? 
            "background-color: #F37321; color: white; font-weight: bold; border-radius: 8px;" : 
            "background-color: #eee; color: black; font-weight: bold; border-radius: 8px;");
        settingIcon->setAlignment(Qt::AlignCenter);
    } else {
        // 버튼 크기에 비례한 아이콘 크기 계산 (버튼 크기의 100%)
        double w_unit = m_parentWidth / 24.0;
        double h_unit = m_parentHeight / 24.0;
        int iconPixelSize = qMin(w_unit * 1.4 * 1.0, h_unit * 1.4 * 1.0);
        
        QPixmap scaledPixmap = settingsPixmap.scaled(iconPixelSize, iconPixelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        settingIcon->setPixmap(scaledPixmap);
        settingIcon->setScaledContents(false);
        settingIcon->setAlignment(Qt::AlignCenter);
        settingIcon->setStyleSheet("border-radius: 8px;");
    }
    
    // 프로필 버튼
    QPixmap profilePixmap = QPixmap(":/images/Profile.png");
    if (profilePixmap.isNull()) profilePixmap = QPixmap("resources/images/Profile.png");
    
    if (profilePixmap.isNull()) {
        loginStatus->clear();
        loginStatus->setText("USER");
        loginStatus->setStyleSheet("background-color: #eee; color: black; font-weight: bold; border-radius: 8px;");
        loginStatus->setAlignment(Qt::AlignCenter);
    } else {
        // 버튼 크기에 비례한 아이콘 크기 계산 (버튼 크기의 100%)
        double w_unit = m_parentWidth / 24.0;
        double h_unit = m_parentHeight / 24.0;
        int iconPixelSize = qMin(w_unit * 1.4 * 1.0, h_unit * 1.4 * 1.0);
        
        QPixmap scaledPixmap = profilePixmap.scaled(iconPixelSize, iconPixelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        loginStatus->setPixmap(scaledPixmap);
        loginStatus->setScaledContents(false);
        loginStatus->setAlignment(Qt::AlignCenter);
        loginStatus->setStyleSheet("border-radius: 8px;");
    }
    
    qDebug() << "버튼 상태 업데이트 완료";
}

void TopBarWidget::updateLayout(int w, int h)
{
    qDebug() << "TopBarWidget 레이아웃 업데이트: " << w << "x" << h;
    
    m_parentWidth = w;
    m_parentHeight = h;
    
    double w_unit = w / 24.0;
    double h_unit = h / 24.0;

    // 로고는 왼쪽에 그대로 (크기 조정)
    logoLabel->setGeometry(w_unit * 1, h_unit * 1, w_unit * 5, h_unit * 2);  // 폭을 조금 늘림
    QFont logoFont = logoLabel->font();
    logoFont.setPointSizeF(h_unit * 0.8);  // 폰트 크기를 조금 더 크게 조정
    logoLabel->setFont(logoFont);

    topLine->setGeometry(w_unit * 1, h_unit * 3 - 1, w_unit * 22, 1);

    // 3개 버튼을 화면 가운데로 배치 (QuadZone 로고와 같은 행에 맞춤)
    double iconSize = w_unit * 1.4;  // 아이콘 크기 (1.2 → 1.4로 증가)
    double iconHeight = h_unit * 1.4;  // 아이콘 높이 (1.2 → 1.4로 증가)
    double iconGap = w_unit * 0.4;  // 아이콘 간격 축소 (0.6 → 0.4)
    double totalButtonWidth = iconSize * 3 + iconGap * 2;  // 3개 버튼 + 2개 간격
    double startX = (w - totalButtonWidth) / 2;  // 화면 가운데 시작점
    double iconY = h_unit * 1.4;  // QuadZone 로고와 같은 행에 맞춤 (로고 중앙 높이)
    
    cameraIcon->setGeometry(startX, iconY, iconSize, iconHeight);
    docIcon->setGeometry(startX + iconSize + iconGap, iconY, iconSize, iconHeight);
    settingIcon->setGeometry(startX + (iconSize + iconGap) * 2, iconY, iconSize, iconHeight);
    
    // 프로필 아이콘은 오른쪽 끝에 (같은 높이로 맞춤)
    loginStatus->setGeometry(w_unit * 21.3, h_unit * 1.4, w_unit * 1.4, h_unit * 1.4);
    
    qDebug() << "버튼 위치 - 시작X:" << startX << "아이콘 크기:" << iconSize;
    
    // 레이아웃 업데이트 후 버튼 상태 다시 업데이트 (이미지 크기 재계산)
    updateButtonStates();
    
    qDebug() << "TopBarWidget 레이아웃 업데이트 완료";
}

void TopBarWidget::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    
    if (getCameraRect().contains(pos)) {
        qDebug() << "카메라 버튼 클릭됨";
        setActiveButton(TopBarButton::Camera);
        emit cameraClicked();
    }
    else if (getDocumentRect().contains(pos)) {
        qDebug() << "문서 버튼 클릭됨";
        setActiveButton(TopBarButton::Document);
        emit documentClicked();
    }
    else if (getSettingsRect().contains(pos)) {
        qDebug() << "설정 버튼 클릭됨";
        setActiveButton(TopBarButton::Settings);
        emit settingsClicked();
    }
    else if (getUserRect().contains(pos)) {
        qDebug() << "사용자 버튼 클릭됨";
        onUserButtonClicked();
    }
    
    QWidget::mousePressEvent(event);
}

QRect TopBarWidget::getCameraRect() const
{
    double w_unit = m_parentWidth / 24.0;
    double h_unit = m_parentHeight / 24.0;
    double iconSize = w_unit * 1.4;  // 크기 업데이트
    double iconHeight = h_unit * 1.4;  // 높이 업데이트
    double iconGap = w_unit * 0.4;  // 간격 축소
    double totalButtonWidth = iconSize * 3 + iconGap * 2;
    double startX = (m_parentWidth - totalButtonWidth) / 2;
    double iconY = h_unit * 1.4;
    
    return QRect(startX, iconY, iconSize, iconHeight);
}

QRect TopBarWidget::getDocumentRect() const
{
    double w_unit = m_parentWidth / 24.0;
    double h_unit = m_parentHeight / 24.0;
    double iconSize = w_unit * 1.4;  // 크기 업데이트
    double iconHeight = h_unit * 1.4;  // 높이 업데이트
    double iconGap = w_unit * 0.4;  // 간격 축소
    double totalButtonWidth = iconSize * 3 + iconGap * 2;
    double startX = (m_parentWidth - totalButtonWidth) / 2;
    double iconY = h_unit * 1.4;
    
    return QRect(startX + iconSize + iconGap, iconY, iconSize, iconHeight);
}

QRect TopBarWidget::getSettingsRect() const
{
    double w_unit = m_parentWidth / 24.0;
    double h_unit = m_parentHeight / 24.0;
    double iconSize = w_unit * 1.4;  // 크기 업데이트
    double iconHeight = h_unit * 1.4;  // 높이 업데이트
    double iconGap = w_unit * 0.4;  // 간격 축소
    double totalButtonWidth = iconSize * 3 + iconGap * 2;
    double startX = (m_parentWidth - totalButtonWidth) / 2;
    double iconY = h_unit * 1.4;
    
    return QRect(startX + (iconSize + iconGap) * 2, iconY, iconSize, iconHeight);
}

QRect TopBarWidget::getUserRect() const
{
    double w_unit = m_parentWidth / 24.0;
    double h_unit = m_parentHeight / 24.0;
    
    return QRect(w_unit * 21.3, h_unit * 1.4, w_unit * 1.4, h_unit * 1.4);
}

void TopBarWidget::setUserEmail(const QString &email)
{
    m_userEmail = email;
}

void TopBarWidget::clearUserData()
{
    m_userEmail.clear();
}

void TopBarWidget::onUserButtonClicked()
{
    qDebug() << "사용자 버튼 클릭됨! 현재 이메일:" << m_userEmail;
    if (!m_userEmail.isEmpty()) {
        showUserMenu();
    } else {
        qDebug() << "이메일이 설정되지 않아 메뉴를 표시하지 않습니다.";
    }
}

void TopBarWidget::showUserMenu()
{
    qDebug() << "showUserMenu 호출됨";
    QMenu *menu = new QMenu(this);
    
    // 외부 클릭 시 메뉴가 자동으로 닫히도록 설정
    menu->setAttribute(Qt::WA_DeleteOnClose);
    
    // QuadZone 주황색 테두리 스타일 설정 (개선된 버전)
    menu->setStyleSheet(
        "QMenu {"
        "   border: 2px solid #F37321;"  // QuadZone 주황색 테두리
        "   background-color: white;"
        "   padding: 8px;"
        "   font-size: 18px;"
        "}"
        "QMenu::item {"
        "   padding: 10px 20px;"
        "   height: 30px;"
        "   border: none;"  // 아이템 테두리 제거
        "}"
        "QMenu::item:selected {"
        "   background-color: transparent;"  // 호버링 배경 제거
        "   color: #001D6C;"
        "}"
        "QMenu::item:disabled {"
        "   color: #F37321;"  // 이메일 글씨를 주황색으로
        "   font-weight: bold;"
        "   font-size: 20px;"
        "   padding: 10px 20px;"
        "   background-color: transparent;"  // 비활성화 항목 배경 투명
        "}"
        "QMenu::item:disabled:selected {"
        "   background-color: transparent;"  // 이메일 항목 호버링 시에도 투명
        "}"
        "QMenu::separator {"
        "   height: 2px;"
        "   background: #cccccc;"
        "   margin: 6px 0px;"
        "}"
    );
    
    // 사용자 이메일 표시 (비활성화된 항목)
    QAction* emailAction = menu->addAction(m_userEmail);
    emailAction->setEnabled(false);  // 클릭 불가능하게 만듦
    menu->addSeparator();
    
    // 로그아웃 액션 추가
    QAction *logoutAction = menu->addAction("로그아웃");
    connect(logoutAction, &QAction::triggered, this, &TopBarWidget::logoutRequested);
    
    // 메뉴 크기를 미리 계산하기 위해 임시로 표시
    menu->adjustSize();
    
    // 메뉴를 사용자 버튼의 왼쪽 아래에 표시
    // 메뉴 너비만큼 왼쪽으로 이동 + 버튼 너비만큼 더 이동하여 완전히 왼쪽에 정렬
    int menuWidth = menu->sizeHint().width();
    int buttonWidth = loginStatus->width();
    QPoint pos = loginStatus->mapToGlobal(QPoint(-menuWidth + buttonWidth, loginStatus->height()));
    qDebug() << "메뉴 표시 위치:" << pos << "메뉴 너비:" << menuWidth << "버튼 너비:" << buttonWidth;
    
    // popup()을 사용하여 외부 클릭 시 자동으로 닫히도록 함
    menu->popup(pos);
    qDebug() << "메뉴 popup 완료";
}
