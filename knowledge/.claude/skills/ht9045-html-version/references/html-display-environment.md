# HT9045 HTML Display Environment

## Target Display

- The HT9045 HTML version runs only on the machine's fixed FullHD display.
- Required resolution: **1920 x 1080**.
- The release runtime is Edge/Chromium kiosk/fullscreen on Windows.
- Fixed VCL-style windows must fit within 1920 x 1080. Where the Windows taskbar is visible, usable height is approximately 1032 px.

## Unsupported Layouts

- Phone, tablet, portrait, and other mobile layouts are outside the product scope.
- Responsive mobile breakpoints, mobile navigation, touch-first rearrangement, and phone/tablet screenshots must not be added unless the machine hardware specification changes.
- Validation must not use phone/tablet viewport sizes such as 390 x 844 as an acceptance criterion.

## Required Validation

- UI acceptance viewport: **1920 x 1080 only**.
- Verify dialogs, fixed windows, text, buttons, and overlays stay inside the FullHD viewport without overlap or clipping.
- Smaller VS Code embedded-browser dimensions may be used only as a tooling limitation check. They do not define product behavior and must not drive layout changes.

## Ownership

This specification applies to `background.html`, `release.html`, `debug.html`, `page/*`, and all HTML dialog overlays. Machine resolution changes require an explicit update to this specification before responsive behavior is introduced.