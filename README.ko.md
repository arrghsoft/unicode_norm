<div align="center" style="text-align: center;">
<h1 style="margin:0; padding: 0;">unicode_norm: Unicode Filename Normalizer</h1>
<div>
    <p>No more "기획서.pdf" turning into "ㄱㅣㅎㅚㄱ서.pdf"</p>
    <p>No more "プログラム.exe" turning into "フ゜ロク゛ラム.pdf"</p>
    <p>No more "résumé.pdf" turning into "re´sume´.pdf"</p>
</div>
<div>
    <p style="margin-bottom: 0.25rem;">Supported Platforms</p>
	<img src="https://img.shields.io/badge/mac%20os-000000?style=for-the-badge&logo=macos&logoColor=F0F0F0" alt=macOS">
    <img src="https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black" alt=Linux">
    <img src="https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white" alt=Windows">
</div>
<div>
    <p style="margin-bottom: 0.25rem;">프로젝트 후원하기</p>
    <a href="https://www.buymeacoffee.com/arrghsoft" target="_blank"><img src="https://www.buymeacoffee.com/assets/img/custom_images/yellow_img.png" alt="Buy Me A Coffee" style="height: 41px !important;width: 174px !important;box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;-webkit-box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;" ></a>
</div>
<p style="text-align: center;" align="center">
  <strong>
    <a href="README.md">English</a> |
    한국어
  </strong>
</p>
<hr>
</div>

# unicode_norm: Unicode Filename Normalizer

파일 이름의 유니코드 정규화(NFC/NFD)를 손쉽게 수행할 수 있게 해주는 CLI 도구입니다.
특히 macOS 환경에서 파일 이름이 자동으로 NFD로 저장되는 문제를 해결하고자 제작되었습니다.

다음 문제를 해결할 수 있습니다.

> Mac에서 '기획서.docx' 파일을 작성하여 Windows를 사용하는 동료에게 보냈는데,
> 파일이 'ㄱㅣㅎㅚㄱ서.docx'로 깨져보이는 거 아니겠습니까? 혹시라도 중요한 거래처에게 보내는 이메일에서 파일 이름이 깨지면 첫인상이 정말 나빠졌을 것 같습니다.

이 문제는 MacOS에서 기본적으로 NFD 방식으로 유니코드 정규화를 수행하기 때문에 발생합니다. Windows나 Linux에서는 NFD가 아닌 NFC 방식으로 파일명을 처리하기 때문에 이런 현상이 발생하는 것입니다. `unicode_norm`을 사용하면 손쉽게 NFC 방식과 NFD 방식으로 파일명을 변환할 수 있습니다.

유니코드 정규화 방식은 다음을 지원합니다.
- NFC (Normalization Form Canonical Composition)
- NFD (Normalization Form Canonical Decomposition)
- NFKC (Normalization Form Compatibility Composition)
- NFKD (Normalization Form Compatibility Decomposition)

## ✨ 주요 기능

- 파일명 및 디렉토리명 유니코드 정규화
- 재귀 처리 (--recursive, -r 옵션, 디렉토리 자동 탐색)
- 테스트 모드 (--dry-run, -d 옵션, 실제로 파일명을 바꾸지는 않는 모드)
- macOS / Linux / Windows 지원

## 📦 설치 방법

Mac에서는 homebrew를 사용하여 다음 명령으로 설치가 가능합니다.

```bash
brew install arrghsoft/tools/unicode_norm
```

명령을 수행했을 때, 나오는 "To enable the Finder Quick Action" 아래의 명령을 직접 터미널에서 실행하면, Finder에서 Quick Action을 통해 파일명을 수정하는 것도 가능하게 됩니다.

```bash
$ brew install arrghsoft/tools/unicode_norm
==> Fetching downloads for: unicode_norm
✔︎ Formula unicode_norm (1.0.1)                                  [Verifying   207.3KB/207.3KB]
==> Installing unicode_norm from arrghsoft/tools
==> cmake . -DUNICODE_NORM_LIBRARY_STATIC_LINK=OFF
==> make install
==> Caveats
To enable the Finder Quick Action:
  cp -r "/opt/homebrew/opt/unicode_norm/share/unicode_norm/Convert to NFC (Windows, Linux).workflow" ~/Library/Services/
  cp -r "/opt/homebrew/opt/unicode_norm/share/unicode_norm/Convert to NFD (Mac).workflow" ~/Library/Services/

unicode_norm is an open source software. You can buy a coffee to the developer via: https://buymeacoffee.com/arrghsoft
==> Summary
🍺  /opt/homebrew/Cellar/unicode_norm/1.0.1: 13 files, 352.6KB, built in 1 second
==> Running `brew cleanup unicode_norm`...
Disable this behaviour by setting `HOMEBREW_NO_INSTALL_CLEANUP=1`.
Hide these hints with `HOMEBREW_NO_ENV_HINTS=1` (see `man brew`).
```

Finder의 Quick Action 스크린샷

![screenshots/quick_action.png](screenshots/quick_action.png)

소스 코드에서 빌드하고 싶은 경우에는 아래의 빌드 관련 안내를 읽어보시기 바랍니다.

Windows에서는 Github의 Release에서 MSI 인스톨러를 다운받으셔서 설치하시기 바랍니다. MSI 인스톨러가 실행파일을 프로그램 폴더에 설치하고, 레지스트리에 마우스 오른쪽 클릭으로 나오는 컨텍스트 메뉴에 Convert to NFC / NFD 항목을 추가합니다.

Linux 환경에서는 아래 빌드 방법을 참고하여 cmake를 이용해서 직접 빌드하여 사용하시기 바랍니다. `linux/install_script.sh`를 실행하여, 마우스 오른쪽 클릭 메뉴에 Convert to NFC / NFD 항목을 추가하실 수 있습니다.

## 🔧 사용 예시

NFC로 정규화
```bash
unicode_norm -f NFC 파일명
```

디렉토리 재귀 변환
```bash
unicode_norm -f NFC -r ~/Downloads/
```

도움말 출력
```bash
unicode_norm -h
```

## 📘 참고

unicode_norm은 utf8proc 라이브러리를 기반으로 정규화를 수행합니다.
HFS, HFS+ 파일시스템을 사용하는 구형 Mac에서는 파일시스템 자체에서 NFD로 저장하기 때문에 작동하지 않을 수 있습니다.
APFS 파일시스템을 사용하는 최근의 Mac에서는 파일시스템에서 NFC, NFD 둘 다 지원하기 때문에 unicode_norm으로 변환이 가능합니다.


## 🚀소스 코드 빌드

라이브러리 정적 링크
```bash
cmake -S . -B build
cmake --build build
```

라이브러리 동적 링크
```bash
cmake -S . -B build -DUNICODE_NORM_LIBRARY_STATIC_LINK=OFF
cmake --build build
```

테스트
```bash
./test.sh
```

설치
```bash
sudo mv build/unicode_norm /usr/local/bin/
```

제거
```bash
sudo rm /usr/local/bin/unicode_norm
```

## 🛠️ 기타

버그 제보 및 신규 기능 요청은 Github Issue를 통해 가능합니다.
