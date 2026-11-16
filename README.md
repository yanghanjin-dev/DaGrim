# DaGrim

Windows용 C / Win32 그림·사진 편집기입니다.

## 설치 및 제거

`DaGrimSetup.exe`를 실행하고 Windows UAC 관리자 권한 요청을 승인하세요.
`Program Files\DaGrim`에 모든 사용자용으로 설치하며 시작 메뉴에 바로가기를 만듭니다.
설치 후 일반 DaGrim 실행에는 관리자 권한이 필요하지 않습니다.
업데이트할 때는 실행 중인 DaGrim을 종료하고 새 설치 파일을 실행하세요.

제거는 Windows 설정의 **앱 → 설치된 앱 → DaGrim → 제거** 또는
설치 폴더의 `DaGrimSetup.exe /uninstall`로 진행합니다. 제거에도 관리자 권한이 필요합니다.
사용자가 만든 문서와 설치 폴더에 추가한 파일은 보존합니다.
기존 단독 실행 파일도 계속 사용할 수 있습니다.
빌드하면 `build\DaGrimSetup.exe`가 함께 생성됩니다.

## 주요 기능

- 최대 64개 레이어, 불투명도 및 8종 혼합 모드
- 브러시·연필·지우개·복제 도장·채우기·그레이디언트
- 사각·타원·올가미·마법봉 선택, 선택 영역 확대·축소·페더
- 문자·도형, 자르기, 레이어 이동·크기 조절·회전
- 밝기·대비·채도·색조, 자동 대비·한계값·포스터화, 흐림·선명화
- `.dagrim` 문서 저장, 이미지 가져오기, PNG/JPEG 내보내기
- 실행 취소/다시 실행, 클립보드, 파일 드래그 앤 드롭

## 빌드

Windows와 MSYS2 UCRT64 GCC / windres가 필요합니다.
`build.bat`는 기본적으로 `C:\msys64\ucrt64\bin`을 사용합니다.

```bat
build.bat
DaGrim.exe
```

## 테스트

```bat
tests\run.bat
```

회전, 색상 보정, 알파 보존, 부분 선택, 선택 영역 처리,
혼합 모드 8종의 문서 저장·재열기를 검증합니다.

## 버전 및 구현 범위

버전의 단일 원본은 `src/version.h`입니다. 개선 작업별로 패치 버전을 올리고
[CHANGELOG.md](CHANGELOG.md)에 변경 사항을 기록합니다.
현재 구현의 제한과 후속 후보는 [DEVELOPMENT.md](DEVELOPMENT.md)를 참고하세요.

## 외부 라이브러리

이미지 입출력에 `stb_image.h`, `stb_image_write.h`를 사용합니다.
각 파일에 포함된 라이선스 고지를 참고하세요.

빌드 결과와 임시 참고 자료는 Git 추적에서 제외합니다.
