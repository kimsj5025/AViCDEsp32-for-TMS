# AViCDEsp32-for-TMS
이 프로젝트는 Thrust Measurement System (TMS)을 위한 AViCDEsp32를 개발한 것으로, 고출력 로켓 모터 개발 과정에서 연소 실험의 추력 측정 및 점화 시스템을 개선하는 데 중점을 두었다.

# 주요 기능
추력 측정 시스템 (TMS) 통합: 기존 TMS의 메인 유닛을 AViCDEsp32로 대체하여 추력 데이터를 효율적으로 측정하고 기록한다.
안정적인 점화: MOSFET을 활용하여 점화 과정을 더욱 안정적으로 제어한다.
원격 제어 및 데이터 로깅: ESP32의 Wi-Fi 기능을 이용하여 추력 데이터 로깅과 점화 활성화를 원격으로 제어하고 데이터를 다운로드한다.
강화된 구조: 로드셀 와이어와 점화 배터리를 위한 박스 구조를 통합하여 시스템의 견고성을 높인다.

# 기술 정보 및 코딩 가이드
AViCDEsp32 프로젝트는 ESP32-S3 마이크로컨트롤러를 활용하며, USB-OTG를 이용해 업로드, 시리얼 모디터 디버깅 등을 지원한다.
ESP32-S3 USB-OTG 지원: ESP32-S3는 USB2.0 풀 스피드를 지원한다.

# USB-OTG를 통한 코드 업로드:
본 코드는 PlatformIO에서 업로드 하기 위한 코드이다. 따라서 아두이노 IDE 에서 사용하기 위해서는 scr/main.cpp 에 있는 코드를 복사하여 사용하면 된다.

# 참고 자료

400Ns High-power 로켓모터 개발일지[https://kimcastleowner.notion.site/AViCDEsp32-20f2a69ddfde80878ec1d9f6c206afdb?source=copy_link]
Esp32-s3에서 USB-OTG로 코딩하는 방법[https://kimsj5025.tistory.com/entry/Esp32-s3에서-USB-OTG로-코딩-하는-방법에-관하여]
