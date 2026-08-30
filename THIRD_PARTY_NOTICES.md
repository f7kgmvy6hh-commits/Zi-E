# Third-Party Notices and Engineering References

## XiaoZhi ESP32

- Project: `78/xiaozhi-esp32`
- Source: https://github.com/78/xiaozhi-esp32
- Pinned engineering reference: `374a5ccf95c2ed513dbad9ca204adbac75062dce`
- Upstream license: MIT; copyright 2025 Shenzhen Xinzhi Future Technology Co., Ltd.
  and Project Contributors.
- Zi-E use in this pass: architecture/runtime patterns and failure lessons referenced;
  no XiaoZhi source or assets copied, no donor snapshot vendored, and no runtime
  dependency added.
- Zi-E modifications: independent contracts and original host-testable Face Engine
  and local safety-voice models implemented behind Zi-E authority boundaries.

The XiaoZhi repository manifest references many separately published components and
assets. Their licenses do not become MIT merely because XiaoZhi is MIT. Any future
component dependency or copied/adapted file must receive its own version, source,
license, notice, compatibility, and modification record before adoption. Face assets
are reviewed independently from code.
