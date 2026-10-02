"""
mark_customer_sections.py
=========================
一次性批次：
  1. 將指定區段標記 audience.customer: true（覆寫該段 YAML）
  2. 補上各語系翻譯（en/zh-TW/vi/ja/ko/id/th）

只處理 TRANSLATIONS dict 內的區段，其他不動。
"""

import os
import re

REFS_DIR = r"d:\HT9045\.github\skills\ht9045-config\references"
DATA_DIR = os.path.join(REFS_DIR, "data")
I18N_DIR = os.path.join(REFS_DIR, "i18n")
LANGS = ['en', 'zh-TW', 'vi', 'ja', 'ko', 'id', 'th']

# ---------------------------------------------------------------------------
# 翻譯資料
#   每個 section 包含 6 種語系的 caption / desc / when / warning / typical
#   shot 為截圖檔名與多語標題（可選）
# ---------------------------------------------------------------------------

TRANSLATIONS = {
    'A02': {
        'group': 'A',
        'related': [],
        'shots': [
            {'file': 'A02-overview.png', 'key': 'A02.shot.overview',
             'cap': {
                'en': 'Bin Setting page with Normal/Prime selection',
                'zh-TW': 'Bin Setting 頁面的 Normal/Prime 切換',
                'vi': 'Trang Bin Setting với lựa chọn Normal/Prime',
                'ja': 'Normal/Prime 切替が表示された Bin Setting 画面',
                'ko': 'Normal/Prime 선택이 표시된 Bin Setting 화면',
                'id': 'Halaman Bin Setting dengan pilihan Normal/Prime',
                'th': 'หน้า Bin Setting พร้อมตัวเลือก Normal/Prime',
             }},
        ],
        'caption': {
            'en': 'Can select Normal or Prime bin data',
            'zh-TW': '可選擇 Normal 或 Prime Bin 資料',
            'vi': 'Có thể chọn dữ liệu Bin Normal hoặc Prime',
            'ja': 'Normal または Prime Bin データを選択可能',
            'ko': 'Normal 또는 Prime Bin 데이터 선택 가능',
            'id': 'Dapat memilih data Bin Normal atau Prime',
            'th': 'สามารถเลือกข้อมูล Bin แบบ Normal หรือ Prime ได้',
        },
        'desc': {
            'en': 'Allows the operator to switch between two independent bin assignment tables (Normal and Prime) without re-loading the recipe.',
            'zh-TW': '允許操作員不重新載入工作檔即可切換兩組獨立的 Bin 對應表（Normal 與 Prime）。',
            'vi': 'Cho phép người vận hành chuyển đổi giữa hai bảng gán bin độc lập (Normal và Prime) mà không cần tải lại recipe.',
            'ja': 'レシピを再読込せずに、Normal と Prime の 2 種類の Bin 割当テーブルを切替えられます。',
            'ko': '레시피를 다시 불러오지 않고도 Normal과 Prime 두 개의 독립적인 Bin 할당 테이블을 전환할 수 있습니다.',
            'id': 'Memungkinkan operator beralih antara dua tabel penetapan bin independen (Normal dan Prime) tanpa memuat ulang resep.',
            'th': 'ให้ผู้ปฏิบัติงานสลับระหว่างตารางกำหนดบินอิสระสองชุด (Normal และ Prime) โดยไม่ต้องโหลดสูตรใหม่',
        },
        'when': {
            'en': 'Enable when the same device requires different bin maps for prime vs. retest runs.',
            'zh-TW': '當相同元件在 Prime 與 Retest 流程需要不同 Bin Map 時啟用。',
            'vi': 'Bật khi cùng một thiết bị cần bản đồ bin khác nhau giữa lần chạy prime và retest.',
            'ja': '同じデバイスで Prime と Retest に異なる Bin Map が必要な場合に有効化します。',
            'ko': '동일 디바이스가 Prime와 Retest 실행에서 다른 Bin Map이 필요할 때 활성화합니다.',
            'id': 'Aktifkan bila perangkat yang sama memerlukan bin map berbeda untuk prime vs retest.',
            'th': 'เปิดใช้งานเมื่ออุปกรณ์เดียวกันต้องการ bin map ที่แตกต่างกันระหว่างการรัน prime และ retest',
        },
        'warning': {
            'en': 'Switching mode does not migrate existing bin counts; counters reset on switch.',
            'zh-TW': '切換模式不會搬移已累積的 Bin 計數；切換時計數會重置。',
            'vi': 'Việc chuyển chế độ không di chuyển số đếm bin hiện có; bộ đếm sẽ được đặt lại khi chuyển.',
            'ja': 'モード切替時、既存の Bin カウントは引き継がれずリセットされます。',
            'ko': '모드 전환 시 기존 Bin 카운트는 이전되지 않고 초기화됩니다.',
            'id': 'Mengganti mode tidak memigrasi hitungan bin yang ada; penghitung akan direset saat beralih.',
            'th': 'การเปลี่ยนโหมดจะไม่ย้ายค่านับ bin ที่มีอยู่ ตัวนับจะถูกรีเซ็ตเมื่อเปลี่ยน',
        },
        'typical': {
            'en': 'Disabled by default; enable only when prime/retest separation is required.',
            'zh-TW': '預設關閉；僅在需要 Prime/Retest 分流時啟用。',
            'vi': 'Tắt theo mặc định; chỉ bật khi cần tách prime/retest.',
            'ja': 'デフォルト無効。Prime/Retest を分離する必要がある場合のみ有効化します。',
            'ko': '기본값은 비활성화. Prime/Retest 분리가 필요할 때만 활성화합니다.',
            'id': 'Dinonaktifkan secara default; aktifkan hanya bila diperlukan pemisahan prime/retest.',
            'th': 'ปิดใช้งานโดยค่าเริ่มต้น เปิดใช้เฉพาะเมื่อต้องการแยก prime/retest เท่านั้น',
        },
    },
    'A05': {
        'group': 'A',
        'related': [],
        'shots': [
            {'file': 'A05-overview.png', 'key': 'A05.shot.overview',
             'cap': {
                'en': 'One Touch Docking (OTD) checkbox in Function tab',
                'zh-TW': 'Function 頁面的 One Touch Docking (OTD) 核取方塊',
                'vi': 'Hộp kiểm One Touch Docking (OTD) trong tab Function',
                'ja': 'Function タブの One Touch Docking (OTD) チェックボックス',
                'ko': 'Function 탭의 One Touch Docking (OTD) 체크박스',
                'id': 'Kotak centang One Touch Docking (OTD) di tab Function',
                'th': 'ช่องทำเครื่องหมาย One Touch Docking (OTD) ในแท็บ Function',
             }},
        ],
        'caption': {
            'en': 'Use one touch docking (OTD)',
            'zh-TW': '使用一鍵對接（OTD）',
            'vi': 'Sử dụng cập bến một chạm (OTD)',
            'ja': 'ワンタッチドッキング (OTD) を使用',
            'ko': '원터치 도킹 (OTD) 사용',
            'id': 'Gunakan one touch docking (OTD)',
            'th': 'ใช้งาน One Touch Docking (OTD)',
        },
        'desc': {
            'en': 'When enabled the handler performs the docking sequence with the test head automatically after pressing a single button.',
            'zh-TW': '啟用後，按下單一按鈕即可自動完成 Handler 與測試頭的對接動作。',
            'vi': 'Khi bật, handler sẽ tự động thực hiện trình tự cập bến với đầu test chỉ sau một lần nhấn nút.',
            'ja': '有効化すると、ボタンを 1 回押すだけでハンドラとテストヘッドのドッキング動作を自動実行します。',
            'ko': '활성화하면 단일 버튼만 눌러도 핸들러와 테스트 헤드의 도킹 시퀀스가 자동 실행됩니다.',
            'id': 'Bila diaktifkan, handler menjalankan urutan docking dengan test head secara otomatis setelah menekan satu tombol.',
            'th': 'เมื่อเปิดใช้งาน Handler จะดำเนินการลำดับ docking กับ test head โดยอัตโนมัติหลังกดปุ่มเดียว',
        },
        'when': {
            'en': 'Use whenever the OTD docking sensors are installed and verified.',
            'zh-TW': '當 OTD 對接感測器安裝並驗證完成後使用。',
            'vi': 'Sử dụng khi cảm biến cập bến OTD đã được lắp đặt và xác minh.',
            'ja': 'OTD ドッキングセンサーを取付け検証済みの場合に使用します。',
            'ko': 'OTD 도킹 센서가 설치 및 검증된 경우에 사용합니다.',
            'id': 'Gunakan bila sensor docking OTD telah terpasang dan diverifikasi.',
            'th': 'ใช้เมื่อติดตั้งและตรวจสอบเซ็นเซอร์ docking OTD เรียบร้อยแล้ว',
        },
        'warning': {
            'en': 'Disable if any docking sensor is faulty—forced docking can damage the test head connector.',
            'zh-TW': '若任一對接感測器故障，請關閉本功能；強制對接可能損壞測試頭連接器。',
            'vi': 'Tắt nếu bất kỳ cảm biến cập bến nào bị hỏng — cập bến cưỡng bức có thể làm hỏng đầu nối test head.',
            'ja': 'ドッキングセンサーに故障がある場合は無効化してください。強制ドッキングはテストヘッドコネクタを破損する恐れがあります。',
            'ko': '도킹 센서가 고장난 경우 비활성화하십시오—강제 도킹은 테스트 헤드 커넥터를 손상시킬 수 있습니다.',
            'id': 'Nonaktifkan jika ada sensor docking yang rusak—docking paksa dapat merusak konektor test head.',
            'th': 'ปิดใช้งานหากเซ็นเซอร์ docking ใดเสีย—การ docking แบบบังคับอาจทำให้ขั้วต่อ test head เสียหาย',
        },
        'typical': {
            'en': 'Enabled in production once installation is complete.',
            'zh-TW': '安裝完成後在量產時啟用。',
            'vi': 'Bật trong sản xuất sau khi hoàn tất cài đặt.',
            'ja': '設置完了後、量産時に有効化します。',
            'ko': '설치 완료 후 양산 시 활성화합니다.',
            'id': 'Diaktifkan dalam produksi setelah pemasangan selesai.',
            'th': 'เปิดใช้งานในการผลิตหลังจากติดตั้งเสร็จสมบูรณ์',
        },
    },
    'A10-1': {
        'group': 'A',
        'related': ['A10-2', 'A10-3', 'A10-4'],
        'shots': [
            {'file': 'A10-art-panel.png', 'key': 'A10-1.shot.overview',
             'cap': {
                'en': 'ART (Auto Retest) configuration panel',
                'zh-TW': 'ART（Auto Retest）設定面板',
                'vi': 'Bảng cấu hình ART (Auto Retest)',
                'ja': 'ART（Auto Retest）設定パネル',
                'ko': 'ART (Auto Retest) 구성 패널',
                'id': 'Panel konfigurasi ART (Auto Retest)',
                'th': 'แผงการกำหนดค่า ART (Auto Retest)',
             }},
        ],
        'caption': {
            'en': 'Enable ART (Auto Retest)',
            'zh-TW': '啟用 ART（自動重測）',
            'vi': 'Bật ART (Tự động test lại)',
            'ja': 'ART（自動リテスト）を有効化',
            'ko': 'ART (자동 재테스트) 활성화',
            'id': 'Aktifkan ART (Pengujian Ulang Otomatis)',
            'th': 'เปิดใช้งาน ART (ทดสอบซ้ำอัตโนมัติ)',
        },
        'desc': {
            'en': 'Automatically reruns failing devices up to a configurable retry limit, separating true defects from contact-related fails.',
            'zh-TW': '自動重測失敗元件至可設定的次數上限，協助區分真正不良與接觸不良。',
            'vi': 'Tự động chạy lại các thiết bị fail tới số lần retry có thể cấu hình, tách lỗi thực sự khỏi lỗi do tiếp xúc.',
            'ja': '不良デバイスを設定可能な回数まで自動再試験し、真の不良と接触不良を切り分けます。',
            'ko': '구성 가능한 재시도 한도까지 불량 디바이스를 자동 재테스트하여 실제 불량과 접촉 불량을 구분합니다.',
            'id': 'Secara otomatis menjalankan ulang perangkat yang fail hingga batas retry yang dapat dikonfigurasi, memisahkan kegagalan asli dari kegagalan kontak.',
            'th': 'ทดสอบอุปกรณ์ที่ fail ซ้ำโดยอัตโนมัติจนถึงขีดจำกัดที่กำหนด เพื่อแยกข้อบกพร่องจริงออกจากปัญหาการสัมผัส',
        },
        'when': {
            'en': 'Enable for devices known to suffer from intermittent contact failures or low first-pass yield.',
            'zh-TW': '當元件容易發生間歇性接觸不良、或首測良率偏低時啟用。',
            'vi': 'Bật cho các thiết bị thường gặp lỗi tiếp xúc gián đoạn hoặc yield lần đầu thấp.',
            'ja': '断続的な接触不良や初回良率が低いデバイスで有効化します。',
            'ko': '간헐적인 접촉 불량 또는 초기 양품률이 낮은 디바이스에 활성화합니다.',
            'id': 'Aktifkan untuk perangkat yang sering mengalami kegagalan kontak intermiten atau yield first-pass rendah.',
            'th': 'เปิดใช้สำหรับอุปกรณ์ที่มีปัญหาการสัมผัสไม่สม่ำเสมอหรือ yield รอบแรกต่ำ',
        },
        'warning': {
            'en': 'ART increases throughput time and may mask socket wear. Set retry limit (A10-2) carefully.',
            'zh-TW': 'ART 會增加產出時間，且可能掩蓋 Socket 老化問題。請審慎設定重測次數 (A10-2)。',
            'vi': 'ART làm tăng thời gian throughput và có thể che giấu hao mòn socket. Hãy đặt giới hạn retry (A10-2) cẩn thận.',
            'ja': 'ART はスループット時間を増加させ、ソケット摩耗を見えなくする可能性があります。リテスト回数 (A10-2) は慎重に設定してください。',
            'ko': 'ART는 처리 시간을 증가시키고 소켓 마모를 가릴 수 있습니다. 재시도 한도 (A10-2)를 신중히 설정하세요.',
            'id': 'ART meningkatkan waktu throughput dan dapat menutupi keausan socket. Atur batas retry (A10-2) dengan hati-hati.',
            'th': 'ART จะเพิ่มเวลาผลผลิตและอาจปิดบังการสึกหรอของ socket โปรดตั้งขีดจำกัด retry (A10-2) อย่างระมัดระวัง',
        },
        'typical': {
            'en': 'Disabled by default. When enabled, retry limit = 1–2 is recommended.',
            'zh-TW': '預設關閉。啟用時建議重測次數 = 1～2。',
            'vi': 'Tắt theo mặc định. Khi bật, khuyến nghị giới hạn retry = 1–2.',
            'ja': 'デフォルト無効。有効化する場合、リテスト回数 = 1〜2 を推奨。',
            'ko': '기본값은 비활성화. 활성화 시 재시도 한도 = 1–2 권장.',
            'id': 'Dinonaktifkan secara default. Jika diaktifkan, batas retry = 1–2 disarankan.',
            'th': 'ปิดใช้งานโดยค่าเริ่มต้น เมื่อเปิดใช้ แนะนำขีดจำกัด retry = 1–2',
        },
    },
    'L04': {
        'group': 'L',
        'related': ['L05', 'L07', 'L08'],
        'shots': [
            {'file': 'L04-overview.png', 'key': 'L04.shot.overview',
             'cap': {
                'en': 'Temperature range setting in Temperature tab',
                'zh-TW': 'Temperature 頁面的 Temperature Range 設定',
                'vi': 'Cài đặt phạm vi nhiệt độ trong tab Temperature',
                'ja': 'Temperature タブの温度範囲設定',
                'ko': 'Temperature 탭의 온도 범위 설정',
                'id': 'Pengaturan rentang suhu di tab Temperature',
                'th': 'การตั้งค่าช่วงอุณหภูมิในแท็บ Temperature',
             }},
        ],
        'caption': {
            'en': 'Test temperature tolerance (±°C, range 2..10)',
            'zh-TW': '測試溫度容許範圍（±°C，範圍 2..10）',
            'vi': 'Dung sai nhiệt độ test (±°C, phạm vi 2..10)',
            'ja': 'テスト温度許容範囲（±°C、範囲 2..10）',
            'ko': '테스트 온도 허용 범위 (±°C, 범위 2..10)',
            'id': 'Toleransi suhu pengujian (±°C, rentang 2..10)',
            'th': 'ช่วงความคลาดเคลื่อนของอุณหภูมิทดสอบ (±°C, ช่วง 2..10)',
        },
        'desc': {
            'en': 'Defines the allowable deviation between actual chamber temperature and target before testing is paused.',
            'zh-TW': '定義實際腔體溫度與目標溫度之間的容許偏差，超過則暫停測試。',
            'vi': 'Xác định sai lệch cho phép giữa nhiệt độ buồng thực tế và mục tiêu trước khi tạm dừng test.',
            'ja': '実際のチャンバー温度と目標温度の許容偏差を定義します。超えるとテストが一時停止します。',
            'ko': '실제 챔버 온도와 목표 온도 간의 허용 편차를 정의합니다. 초과 시 테스트가 일시 중지됩니다.',
            'id': 'Mendefinisikan deviasi yang diizinkan antara suhu chamber aktual dan target sebelum pengujian dijeda.',
            'th': 'กำหนดค่าเบี่ยงเบนที่ยอมรับได้ระหว่างอุณหภูมิ chamber จริงกับเป้าหมายก่อนหยุดทดสอบชั่วคราว',
        },
        'when': {
            'en': 'Tighten for high-precision devices; loosen if chamber stability is limited.',
            'zh-TW': '高精度元件可調緊；若腔體穩定度受限可調寬。',
            'vi': 'Thắt chặt cho thiết bị độ chính xác cao; nới lỏng nếu độ ổn định buồng bị giới hạn.',
            'ja': '高精度デバイスでは厳しく、チャンバー安定性に制約がある場合は緩めに設定します。',
            'ko': '고정밀 디바이스에서는 좁게, 챔버 안정성이 제한적이면 넓게 설정합니다.',
            'id': 'Perketat untuk perangkat presisi tinggi; longgarkan bila stabilitas chamber terbatas.',
            'th': 'ตั้งให้แคบสำหรับอุปกรณ์ความแม่นยำสูง; ตั้งให้กว้างหากเสถียรภาพ chamber จำกัด',
        },
        'warning': {
            'en': 'Setting below 2 °C may cause excessive pause/resume cycles and reduce throughput.',
            'zh-TW': '設定低於 2 °C 可能造成頻繁的暫停／恢復循環並降低產出。',
            'vi': 'Thiết lập dưới 2 °C có thể gây ra chu kỳ tạm dừng/tiếp tục quá mức và giảm throughput.',
            'ja': '2 °C 未満に設定するとポーズ/再開が頻発し、スループットが低下する可能性があります。',
            'ko': '2 °C 미만으로 설정하면 일시 중지/재개 주기가 과도해져 처리량이 감소할 수 있습니다.',
            'id': 'Pengaturan di bawah 2 °C dapat menyebabkan siklus pause/resume berlebihan dan menurunkan throughput.',
            'th': 'การตั้งค่าต่ำกว่า 2 °C อาจทำให้เกิดวงจร pause/resume มากเกินไปและลดผลผลิต',
        },
        'typical': {
            'en': 'Default 3 °C for most devices.',
            'zh-TW': '大多數元件預設 3 °C。',
            'vi': 'Mặc định 3 °C cho hầu hết thiết bị.',
            'ja': '多くのデバイスで既定 3 °C。',
            'ko': '대부분의 디바이스에서 기본값 3 °C.',
            'id': 'Default 3 °C untuk sebagian besar perangkat.',
            'th': 'ค่าเริ่มต้น 3 °C สำหรับอุปกรณ์ส่วนใหญ่',
        },
    },
    'N06': {
        'group': 'N',
        'related': ['N07-1'],
        'shots': [
            {'file': 'N06-ftp-config.png', 'key': 'N06.shot.overview',
             'cap': {
                'en': 'FTP recipe upload/download settings',
                'zh-TW': 'FTP 工作檔上傳／下載設定',
                'vi': 'Cài đặt tải lên/tải xuống recipe qua FTP',
                'ja': 'FTP レシピアップロード/ダウンロード設定',
                'ko': 'FTP 레시피 업로드/다운로드 설정',
                'id': 'Pengaturan upload/download resep FTP',
                'th': 'การตั้งค่าอัปโหลด/ดาวน์โหลดสูตรผ่าน FTP',
             }},
        ],
        'caption': {
            'en': 'FTP setting for recipe file (upload/download)',
            'zh-TW': 'FTP 工作檔上傳／下載設定',
            'vi': 'Cài đặt FTP cho tệp recipe (tải lên/tải xuống)',
            'ja': 'レシピファイルの FTP 設定（アップロード/ダウンロード）',
            'ko': '레시피 파일 FTP 설정 (업로드/다운로드)',
            'id': 'Pengaturan FTP untuk file resep (upload/download)',
            'th': 'การตั้งค่า FTP สำหรับไฟล์สูตร (อัปโหลด/ดาวน์โหลด)',
        },
        'desc': {
            'en': 'Synchronizes recipe files between the handler and a central FTP server, allowing centralized recipe management across multiple machines.',
            'zh-TW': '在 Handler 與中央 FTP 伺服器之間同步工作檔，達成跨機台的集中式工作檔管理。',
            'vi': 'Đồng bộ các tệp recipe giữa handler và máy chủ FTP trung tâm, cho phép quản lý recipe tập trung trên nhiều máy.',
            'ja': 'ハンドラと中央 FTP サーバ間でレシピファイルを同期し、複数機台のレシピを集中管理できます。',
            'ko': '핸들러와 중앙 FTP 서버 간 레시피 파일을 동기화하여 여러 장비의 레시피를 중앙에서 관리할 수 있습니다.',
            'id': 'Menyinkronkan file resep antara handler dan server FTP pusat, memungkinkan manajemen resep terpusat di beberapa mesin.',
            'th': 'ซิงโครไนซ์ไฟล์สูตรระหว่าง handler และเซิร์ฟเวอร์ FTP ส่วนกลาง เพื่อจัดการสูตรจากศูนย์กลางในหลายเครื่อง',
        },
        'when': {
            'en': 'Enable in fab environments where recipes must remain consistent across many handlers.',
            'zh-TW': '當廠房需要多台 Handler 共用一致的工作檔時啟用。',
            'vi': 'Bật trong môi trường fab nơi recipe phải nhất quán trên nhiều handler.',
            'ja': '複数のハンドラ間でレシピを統一する必要がある工場環境で有効化します。',
            'ko': '여러 핸들러 간 레시피 일관성이 필요한 팹 환경에서 활성화합니다.',
            'id': 'Aktifkan di lingkungan fab di mana resep harus konsisten di banyak handler.',
            'th': 'เปิดใช้ในสภาพแวดล้อม fab ที่สูตรต้องสอดคล้องกันในหลาย handler',
        },
        'warning': {
            'en': 'Verify FTP credentials and network reachability before enabling production runs.',
            'zh-TW': '在量產前請先驗證 FTP 帳密與網路可達性。',
            'vi': 'Xác minh thông tin FTP và khả năng kết nối mạng trước khi chạy sản xuất.',
            'ja': '量産開始前に FTP の認証情報とネットワーク疎通を確認してください。',
            'ko': '양산 시작 전 FTP 자격 증명과 네트워크 도달 가능성을 확인하세요.',
            'id': 'Verifikasi kredensial FTP dan jangkauan jaringan sebelum mengaktifkan run produksi.',
            'th': 'ตรวจสอบข้อมูลการเข้าถึง FTP และการเข้าถึงเครือข่ายก่อนเปิดใช้การรันการผลิต',
        },
        'typical': {
            'en': 'Disabled by default; enabled only after IT-approved FTP server is provisioned.',
            'zh-TW': '預設關閉；僅在 IT 核可的 FTP 伺服器佈署後啟用。',
            'vi': 'Tắt theo mặc định; chỉ bật sau khi máy chủ FTP được IT phê duyệt và triển khai.',
            'ja': 'デフォルト無効。IT 承認済の FTP サーバ構築後にのみ有効化します。',
            'ko': '기본값은 비활성화. IT 승인된 FTP 서버 구축 후에만 활성화합니다.',
            'id': 'Dinonaktifkan secara default; diaktifkan hanya setelah server FTP yang disetujui IT tersedia.',
            'th': 'ปิดใช้โดยค่าเริ่มต้น เปิดใช้เฉพาะหลังจากตั้งค่าเซิร์ฟเวอร์ FTP ที่ IT อนุมัติแล้ว',
        },
    },
    'N07-1': {
        'group': 'N',
        'related': [],
        'shots': [
            {'file': 'N07-secs-gem.png', 'key': 'N07-1.shot.overview',
             'cap': {
                'en': 'Enable SECS/GEM checkbox in Network tab',
                'zh-TW': 'Network 頁面的 Enable SECS/GEM 核取方塊',
                'vi': 'Hộp kiểm Bật SECS/GEM trong tab Network',
                'ja': 'Network タブの SECS/GEM 有効化チェックボックス',
                'ko': 'Network 탭의 SECS/GEM 활성화 체크박스',
                'id': 'Kotak centang Aktifkan SECS/GEM di tab Network',
                'th': 'ช่องทำเครื่องหมายเปิดใช้ SECS/GEM ในแท็บ Network',
             }},
        ],
        'caption': {
            'en': 'Enable SECS/GEM communication',
            'zh-TW': '啟用 SECS/GEM 通訊',
            'vi': 'Bật giao tiếp SECS/GEM',
            'ja': 'SECS/GEM 通信を有効化',
            'ko': 'SECS/GEM 통신 활성화',
            'id': 'Aktifkan komunikasi SECS/GEM',
            'th': 'เปิดใช้การสื่อสาร SECS/GEM',
        },
        'desc': {
            'en': 'Activates the SEMI E5/E30 (SECS/GEM) interface so the handler can be controlled and monitored by the host MES system.',
            'zh-TW': '啟用 SEMI E5/E30（SECS/GEM）介面，讓 Handler 可由 MES 主機控制與監控。',
            'vi': 'Kích hoạt giao diện SEMI E5/E30 (SECS/GEM) để handler có thể được điều khiển và giám sát bởi hệ thống MES máy chủ.',
            'ja': 'SEMI E5/E30（SECS/GEM）インターフェイスを有効化し、ホスト MES からハンドラを制御・監視できるようにします。',
            'ko': 'SEMI E5/E30 (SECS/GEM) 인터페이스를 활성화하여 호스트 MES 시스템에서 핸들러를 제어 및 모니터링할 수 있게 합니다.',
            'id': 'Mengaktifkan antarmuka SEMI E5/E30 (SECS/GEM) sehingga handler dapat dikendalikan dan dimonitor oleh sistem MES host.',
            'th': 'เปิดใช้งานอินเทอร์เฟซ SEMI E5/E30 (SECS/GEM) เพื่อให้ระบบ MES โฮสต์ควบคุมและตรวจสอบ handler ได้',
        },
        'when': {
            'en': 'Required when integrating the handler into a SEMI-compliant MES or factory automation environment.',
            'zh-TW': '當 Handler 需整合至 SEMI 規範的 MES 或工廠自動化環境時必須啟用。',
            'vi': 'Bắt buộc khi tích hợp handler vào MES tuân thủ SEMI hoặc môi trường tự động hóa nhà máy.',
            'ja': 'SEMI 準拠の MES や工場自動化環境にハンドラを統合する場合に必要です。',
            'ko': '핸들러를 SEMI 호환 MES 또는 공장 자동화 환경에 통합할 때 필요합니다.',
            'id': 'Diperlukan saat mengintegrasikan handler ke MES sesuai SEMI atau lingkungan otomasi pabrik.',
            'th': 'จำเป็นเมื่อรวม handler เข้ากับ MES ที่เป็นไปตาม SEMI หรือสภาพแวดล้อมอัตโนมัติของโรงงาน',
        },
        'warning': {
            'en': 'GemSys.ini must be configured with the correct host IP/port and device ID before enabling—incorrect values will block recipe operations.',
            'zh-TW': '啟用前必須先在 GemSys.ini 設定正確的主機 IP／Port 與裝置 ID；設定錯誤會阻擋工作檔操作。',
            'vi': 'GemSys.ini phải được cấu hình IP/cổng máy chủ và ID thiết bị chính xác trước khi bật — giá trị sai sẽ chặn các thao tác recipe.',
            'ja': '有効化前に GemSys.ini にホスト IP/ポートとデバイス ID を正しく設定してください。誤設定はレシピ操作をブロックします。',
            'ko': '활성화 전에 GemSys.ini에 호스트 IP/포트 및 장치 ID를 올바르게 구성해야 합니다. 잘못된 값은 레시피 동작을 차단합니다.',
            'id': 'GemSys.ini harus dikonfigurasi dengan IP/port host dan ID perangkat yang benar sebelum diaktifkan—nilai salah akan memblokir operasi resep.',
            'th': 'ต้องกำหนดค่า GemSys.ini ด้วย IP/พอร์ตโฮสต์และรหัสอุปกรณ์ที่ถูกต้องก่อนเปิดใช้ ค่าที่ผิดจะบล็อกการทำงานของสูตร',
        },
        'typical': {
            'en': 'Disabled by default; enable only after host commissioning is complete.',
            'zh-TW': '預設關閉；僅在主機連線測試完成後啟用。',
            'vi': 'Tắt theo mặc định; chỉ bật sau khi commissioning máy chủ hoàn tất.',
            'ja': 'デフォルト無効。ホスト接続試験完了後にのみ有効化します。',
            'ko': '기본값은 비활성화. 호스트 커미셔닝 완료 후에만 활성화합니다.',
            'id': 'Dinonaktifkan secara default; aktifkan hanya setelah commissioning host selesai.',
            'th': 'ปิดใช้โดยค่าเริ่มต้น เปิดใช้เฉพาะหลังจาก commissioning โฮสต์เสร็จสมบูรณ์',
        },
    },
    'L43': {
        'group': 'L',
        'related': ['L11', 'L23'],
        'shots': [
            {'file': 'L43-overview.png', 'key': 'L43.shot.overview',
             'cap': {
                'en': 'PowerFollow checkbox in the Temperature settings tab',
                'zh-TW': '溫度設定頁的 PowerFollow 核取方塊',
                'vi': 'Hộp kiểm PowerFollow trong tab cài đặt Temperature',
                'ja': 'Temperature 設定タブの PowerFollow チェックボックス',
                'ko': 'Temperature 설정 탭의 PowerFollow 체크박스',
                'id': 'Kotak centang PowerFollow di tab pengaturan Temperature',
                'th': 'ช่องทำเครื่องหมาย PowerFollow ในแท็บการตั้งค่า Temperature',
             }},
            {'file': 'L43-enabled.png', 'key': 'L43.shot.enabled',
             'cap': {
                'en': 'GroupBox shown when PowerFollow is enabled',
                'zh-TW': 'PowerFollow 啟用時顯示的 GroupBox',
                'vi': 'GroupBox hiển thị khi PowerFollow được bật',
                'ja': 'PowerFollow 有効時に表示される GroupBox',
                'ko': 'PowerFollow 활성화 시 표시되는 GroupBox',
                'id': 'GroupBox yang ditampilkan saat PowerFollow diaktifkan',
                'th': 'GroupBox ที่แสดงเมื่อ PowerFollow เปิดใช้งาน',
             }},
        ],
        'caption': {
            'en': 'Enable Power Follow Function',
            'zh-TW': '啟用 Power Follow 功能',
            'vi': 'Bật tính năng Power Follow',
            'ja': 'Power Follow 機能を有効化',
            'ko': 'Power Follow 기능 활성화',
            'id': 'Aktifkan Fungsi Power Follow',
            'th': 'เปิดใช้งานฟังก์ชัน Power Follow',
        },
        'desc': {
            'en': 'When enabled, the chamber heater tracks the IC junction temperature in real time, ensuring stable test temperature even with high power devices.',
            'zh-TW': '啟用後，溫控腔的加熱會即時追隨 IC 接面溫度，確保高功率元件測試時溫度穩定。',
            'vi': 'Khi bật, bộ gia nhiệt của buồng theo dõi nhiệt độ tiếp giáp IC theo thời gian thực, đảm bảo nhiệt độ test ổn định ngay cả với thiết bị công suất cao.',
            'ja': '有効化すると、チャンバーヒーターが IC 接合部温度をリアルタイムで追従し、高消費電力デバイスでも安定した試験温度を確保します。',
            'ko': '활성화하면 챔버 히터가 IC 접합부 온도를 실시간으로 추적하여 고출력 디바이스에서도 안정적인 테스트 온도를 보장합니다.',
            'id': 'Bila diaktifkan, pemanas chamber melacak suhu junction IC secara real-time, memastikan suhu pengujian stabil bahkan untuk perangkat berdaya tinggi.',
            'th': 'เมื่อเปิดใช้งาน เครื่องทำความร้อน chamber จะติดตามอุณหภูมิ junction ของ IC แบบเรียลไทม์ ทำให้อุณหภูมิทดสอบเสถียรแม้ในอุปกรณ์กำลังสูง',
        },
        'when': {
            'en': 'Use this option for high-power devices (>2W) where standard ATC control cannot keep the IC at the target temperature.',
            'zh-TW': '當測試高功率元件（>2W），標準 ATC 控制無法將 IC 維持在目標溫度時啟用。',
            'vi': 'Sử dụng tùy chọn này cho các thiết bị công suất cao (>2W) khi điều khiển ATC tiêu chuẩn không giữ được IC ở nhiệt độ mục tiêu.',
            'ja': '標準 ATC 制御で IC を目標温度に維持できない高消費電力デバイス（>2W）で使用します。',
            'ko': '표준 ATC 제어로 IC를 목표 온도로 유지할 수 없는 고출력 디바이스 (>2W)에서 사용합니다.',
            'id': 'Gunakan opsi ini untuk perangkat berdaya tinggi (>2W) di mana kontrol ATC standar tidak dapat menjaga IC pada suhu target.',
            'th': 'ใช้ตัวเลือกนี้สำหรับอุปกรณ์กำลังสูง (>2W) ที่ระบบควบคุม ATC มาตรฐานไม่สามารถรักษา IC ให้อยู่ที่อุณหภูมิเป้าหมายได้',
        },
        'warning': {
            'en': 'Power Follow requires a calibrated TJ Slope/Offset for the device. Without calibration the temperature may drift and cause yield loss.',
            'zh-TW': 'Power Follow 需要事先完成元件 TJ Slope/Offset 校正，未校正會導致溫度偏移與良率損失。',
            'vi': 'Power Follow yêu cầu hiệu chuẩn TJ Slope/Offset cho thiết bị. Không hiệu chuẩn có thể gây trôi nhiệt độ và mất yield.',
            'ja': 'Power Follow にはデバイスの TJ Slope/Offset 校正が必要です。未校正の場合、温度ドリフトと歩留り損失を招く可能性があります。',
            'ko': 'Power Follow는 디바이스의 TJ Slope/Offset 보정이 필요합니다. 보정 없이 사용 시 온도 드리프트와 양품률 손실이 발생할 수 있습니다.',
            'id': 'Power Follow memerlukan kalibrasi TJ Slope/Offset untuk perangkat. Tanpa kalibrasi, suhu dapat bergeser dan menyebabkan kehilangan yield.',
            'th': 'Power Follow ต้องการการคาลิเบรต TJ Slope/Offset สำหรับอุปกรณ์ หากไม่คาลิเบรตอาจทำให้อุณหภูมิเลื่อนและสูญเสีย yield',
        },
        'typical': {
            'en': 'Disabled by default. Enable only after thermal calibration is verified.',
            'zh-TW': '預設關閉。完成熱校正驗證後再啟用。',
            'vi': 'Tắt theo mặc định. Chỉ bật sau khi xác minh hiệu chuẩn nhiệt.',
            'ja': 'デフォルト無効。熱校正の検証後にのみ有効化します。',
            'ko': '기본값은 비활성화. 열 보정 검증 후에만 활성화합니다.',
            'id': 'Dinonaktifkan secara default. Aktifkan hanya setelah kalibrasi termal diverifikasi.',
            'th': 'ปิดใช้โดยค่าเริ่มต้น เปิดใช้เฉพาะหลังตรวจสอบการคาลิเบรตความร้อนแล้ว',
        },
    },
}


# ---------------------------------------------------------------------------
# 工具
# ---------------------------------------------------------------------------

def yaml_escape(s):
    if s is None:
        return ''
    s = str(s)
    if any(c in s for c in [':', '#', '"', '\\', "'", '\n']) or \
       s.strip() != s or s in ('true', 'false', 'null', ''):
        return '"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"'
    return s


def patch_yaml(section, info):
    path = os.path.join(DATA_DIR, info['group'], f"{section}.yaml")
    if not os.path.exists(path):
        print(f"  [WARN] {path} not found, skip")
        return False
    with open(path, encoding='utf-8') as f:
        text = f.read()

    # 1. audience.customer: false → true
    text = re.sub(r'(audience:\s*\n(?:  [^\n]+\n)*?  customer:\s*)false',
                  r'\1true', text)

    # 2. related_sections: [] → list
    if info.get('related'):
        related_block = 'related_sections:\n' + '\n'.join(
            f'  - {r}' for r in info['related'])
        text = re.sub(r'related_sections:\s*\[\]', related_block, text, count=1)

    # 3. screenshots: [] → list with files
    if info.get('shots'):
        shots_block = 'screenshots:\n'
        for s in info['shots']:
            shots_block += f"  -\n    file: {s['file']}\n    caption_id: {s['key']}\n"
        shots_block = shots_block.rstrip('\n')
        text = re.sub(r'screenshots:\s*\[\]', shots_block, text, count=1)

    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(text)
    return True


def patch_i18n(section, info):
    """Update translation keys for all languages."""
    fields = ['caption', 'desc', 'when', 'warning', 'typical']
    suffix_map = {
        'caption': 'caption',
        'desc': 'desc',
        'when': 'when',
        'warning': 'warning',
        'typical': 'typical',
    }

    for lang in LANGS:
        path = os.path.join(I18N_DIR, f'{lang}.yaml')
        if not os.path.exists(path):
            continue
        with open(path, encoding='utf-8') as f:
            lines = f.readlines()

        for fld in fields:
            if fld not in info:
                continue
            new_val = info[fld].get(lang, info[fld].get('en', ''))
            key = f'{section}.{suffix_map[fld]}'
            replaced = False
            for i, ln in enumerate(lines):
                if ln.startswith(key + ':'):
                    lines[i] = f'{key}: {yaml_escape(new_val)}\n'
                    replaced = True
                    break
            if not replaced:
                lines.append(f'{key}: {yaml_escape(new_val)}\n')

        # 處理 shot caption
        for s in info.get('shots', []):
            key = s['key']
            cap = s['cap'].get(lang, s['cap'].get('en', ''))
            replaced = False
            for i, ln in enumerate(lines):
                if ln.startswith(key + ':'):
                    lines[i] = f'{key}: {yaml_escape(cap)}\n'
                    replaced = True
                    break
            if not replaced:
                lines.append(f'{key}: {yaml_escape(cap)}\n')

        with open(path, 'w', encoding='utf-8', newline='\n') as f:
            f.writelines(lines)


def main():
    print("=" * 60)
    print(f"Mark customer sections + translate ({len(TRANSLATIONS)} sections)")
    print("=" * 60)
    for section, info in TRANSLATIONS.items():
        ok = patch_yaml(section, info)
        if not ok:
            continue
        patch_i18n(section, info)
        print(f"  [{section}] YAML + 7-lang i18n updated")
    print("\nDone.")


if __name__ == '__main__':
    main()
