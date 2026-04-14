# 🤖 Multi-Robot-Logistics-System

**Gazebo 시뮬레이션 기반의 다기종 로봇 군집 제어 및 물류 자동화 시스템**입니다.  
서로 다른 기종의 로봇(TurtleBot3, Push Robot)이 협업하여 하역부터 이송까지의 전 과정을 자율적으로 수행하며, ROS2의 네임스페이스와 통신 최적화를 통해 실시간 군집 제어 안정성을 확보했습니다.

![ROS2](https://img.shields.io/badge/ROS2-Humble-blue?style=flat&logo=ros)
![Python](https://img.shields.io/badge/Python-3776AB?style=flat&logo=python&logoColor=white)
![C++](https://img.shields.io/badge/C++-00599C?style=flat&logo=cplusplus&logoColor=white)
![Gazebo](https://img.shields.io/badge/Gazebo-FFD43B?style=flat&logo=gazebo&logoColor=black)

| 항목 | 내용 |
|------|------|
| 참여 인원 | 4명 (팀장 / PM 역할 수행) |
| 진행 기간 | 2023.05 – 2023.06 |
| 사용 로봇 | TurtleBot3 Burger/Waffle, Custom Push Robot |
| 주요 기술 | Multi-robot Coordination, Namespace Management, Gazebo Modeling |

---

## 📌 개요

가상 물류 창고 환경을 Gazebo에서 직접 모델링하고, 다수의 로봇이 병목 현상 없이 협업할 수 있는 제어 시스템을 구축했습니다. 특히 다기종 로봇 간의 데이터 혼선을 방지하기 위한 통신 구조 설계와 효율적인 작업 분배 알고리즘 구현에 집중했습니다.

---

## 🏗 시스템 구조 (System Architecture)

다수의 로봇을 안정적으로 제어하기 위해 **분산형 토픽 구조**를 설계했습니다.

1.  **Environment:** Gazebo 기반의 맞춤형 물류 창고 월드(World) 구성
2.  **Perception:** 각 로봇의 LiDAR 및 센서 데이터를 통한 개별 장애물 회피
3.  **Coordination:** 중앙 노드에서 각 로봇(Namespace 분리)에 작업 할당 및 상태 모니터링
4.  **Task Flow:** Push Robot(하역) → TurtleBot(이송) 간의 상호작용 로직

---

## ⚙️ 주요 구현 내용

- **다기종 로봇 환경 구축** — Gazebo 내에 물리 계수가 다른 TurtleBot3와 Push Robot을 모델링하고 물리적 상호작용 구현
- **ROS2 네임스페이스 최적화** — 다수 로봇 운용 시 발생하는 데이터 병목을 방지하기 위해 토픽 네임스페이스를 체계적으로 관리
- **군집 제어 알고리즘** — 다수의 로봇이 충돌 없이 목표 지점까지 이동할 수 있는 경로 추종 및 충돌 방지 로직 적용
- **실시간 모니터링** — RViz2를 활용하여 여러 로봇의 센서 데이터와 현재 상태를 통합 시각화

---

## 🚀 기술적 도전 및 해결 (Troubleshooting)

### 1. 통신 지연 및 데이터 혼선 해결
- **문제:** 로봇 대수가 늘어남에 따라 동일한 이름의 토픽들이 충돌하거나 통신 부하가 발생함.
- **해결:** ROS2의 **Namespace** 기능을 활용하여 노드와 토픽을 완전 분리하고, 필요한 고성능 연산은 C++ 노드로 구현하여 통신 효율을 30% 이상 개선함.

### 2. 다기종 로봇 간 협업 로직
- **문제:** 로봇마다 이동 속도와 회전 반경이 달라 협업 타이밍이 어긋남.
- **해결:** 작업 상태를 공유하는 **State Machine**을 도입하여 하역 작업 완료 신호(Topic)를 받은 이송 로봇만 출발하도록 동기화 구현.

---

## 🎬 시연 영상
(여기에 시연 영상 링크를 삽입하세요)

| 가상 환경 모델링 | 다기종 로봇 협업 | 실시간 경로 모니터링 |
|:---:|:---:|:---:|
| ![env](./assets/gazebo_world.png) | ![collab](./assets/multi_robot.png) | ![rviz](./assets/rviz_view.png) |

---

## 📂 프로젝트 구조

```
├── models/               # 직접 모델링한 Gazebo 로봇 및 환경 파일(.sdf, .world)
├── worlds/               # 물류 창고 월드 설정
├── launch/               # 다기종 로봇 통합 실행 스크립트
├── src/
│   ├── fleet_manager.py  # 전체 로봇 작업 할당 및 관리
│   ├── navigation.cpp    # 고속 경로 추종 제어기
│   └── robot_control/    # 개별 로봇 제어 노드
└── README.md
```
