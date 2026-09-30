'use strict';
// AI(W906-HTDESIGNER) 20260930: 事件表 -- the events a VCL class has, like the Events tab of BCB6's Object
// Inspector / WPF's Properties window (EastSun: "屬性裡面的事件我希望可以像WPF一樣有很多表格事件，如果有設定事件
// 上面就會顯示是什麼函式，並且雙擊兩下可以跑到對應CODE，如果沒有設定就會自動新增相關設定").
// The lists are BCB6's (VCL 6) published events of the classes these forms use; a class not listed gets the
// TControl ones, and whatever the .dfm files assign for it is added (eventsOf's `extra`). Plain Node.

const MOUSE = ['OnClick', 'OnDblClick', 'OnMouseDown', 'OnMouseMove', 'OnMouseUp'];
const FOCUS = ['OnEnter', 'OnExit', 'OnKeyDown', 'OnKeyPress', 'OnKeyUp'];
const DRAG = ['OnContextPopup', 'OnDragDrop', 'OnDragOver', 'OnEndDrag', 'OnStartDrag'];
const DOCK = ['OnEndDock', 'OnStartDock'];
const WIN = MOUSE.concat(FOCUS, DRAG, DOCK);

const BY = {
  TForm: ['OnActivate', 'OnCanResize', 'OnClick', 'OnClose', 'OnCloseQuery', 'OnConstrainedResize', 'OnContextPopup', 'OnCreate',
    'OnDblClick', 'OnDeactivate', 'OnDestroy', 'OnDockDrop', 'OnDockOver', 'OnDragDrop', 'OnDragOver', 'OnGetSiteInfo', 'OnHelp',
    'OnHide', 'OnKeyDown', 'OnKeyPress', 'OnKeyUp', 'OnMouseDown', 'OnMouseMove', 'OnMouseUp', 'OnMouseWheel',
    'OnMouseWheelDown', 'OnMouseWheelUp', 'OnPaint', 'OnResize', 'OnShortCut', 'OnShow', 'OnUnDock'],
  TButton: WIN.filter(e => e !== 'OnDblClick'),
  TBitBtn: WIN.filter(e => e !== 'OnDblClick'),
  TSpeedButton: MOUSE,
  TLabel: MOUSE.concat(DRAG, ['OnMouseEnter', 'OnMouseLeave']),
  TStaticText: MOUSE.concat(DRAG),
  TEdit: WIN.concat(['OnChange']),
  TMaskEdit: WIN.concat(['OnChange']),
  TMemo: WIN.concat(['OnChange']),
  TRichEdit: WIN.concat(['OnChange', 'OnSelectionChange', 'OnProtectChange', 'OnResizeRequest', 'OnSaveClipboard']),
  TCSpinEdit: WIN.concat(['OnChange']),
  TCheckBox: WIN.filter(e => e !== 'OnDblClick'),
  TRadioButton: WIN,
  TComboBox: FOCUS.concat(DRAG, DOCK, ['OnChange', 'OnClick', 'OnCloseUp', 'OnDblClick', 'OnDrawItem', 'OnDropDown', 'OnMeasureItem', 'OnSelect']),
  TListBox: WIN.concat(['OnDrawItem', 'OnMeasureItem']),
  TRadioGroup: ['OnClick', 'OnContextPopup', 'OnDragDrop', 'OnDragOver', 'OnEndDock', 'OnEndDrag', 'OnEnter', 'OnExit', 'OnStartDock', 'OnStartDrag'],
  TGroupBox: WIN.filter(e => !/^OnKey/.test(e)),
  TPanel: WIN.filter(e => !/^OnKey/.test(e)).concat(['OnCanResize', 'OnConstrainedResize', 'OnDockDrop', 'OnDockOver', 'OnGetSiteInfo', 'OnResize', 'OnUnDock']),
  TPageControl: WIN.filter(e => e !== 'OnClick' && e !== 'OnDblClick').concat(['OnChange', 'OnChanging', 'OnDockDrop', 'OnDockOver', 'OnDrawTab', 'OnGetImageIndex', 'OnGetSiteInfo', 'OnResize', 'OnUnDock']),
  TTabSheet: ['OnContextPopup', 'OnDragDrop', 'OnDragOver', 'OnEndDrag', 'OnEnter', 'OnExit', 'OnHide', 'OnMouseDown', 'OnMouseMove', 'OnMouseUp', 'OnResize', 'OnShow', 'OnStartDrag'],
  TTimer: ['OnTimer'],
  TTrackBar: FOCUS.concat(DRAG, DOCK, ['OnChange']),
  TScrollBar: FOCUS.concat(DRAG, DOCK, ['OnChange', 'OnScroll']),
  TUpDown: ['OnChanging', 'OnChangingEx', 'OnClick', 'OnContextPopup', 'OnEnter', 'OnExit', 'OnMouseDown', 'OnMouseMove', 'OnMouseUp'],
  TImage: MOUSE.concat(DRAG, ['OnProgress']),
  TShape: MOUSE.concat(DRAG),
  TPaintBox: MOUSE.concat(DRAG, ['OnPaint']),
  TBevel: [],
  TStringGrid: WIN.concat(['OnColumnMoved', 'OnDrawCell', 'OnGetEditMask', 'OnGetEditText', 'OnRowMoved', 'OnSelectCell', 'OnSetEditText', 'OnTopLeftChanged', 'OnMouseWheelDown', 'OnMouseWheelUp']),
  TDrawGrid: WIN.concat(['OnColumnMoved', 'OnDrawCell', 'OnGetEditMask', 'OnGetEditText', 'OnRowMoved', 'OnSelectCell', 'OnSetEditText', 'OnTopLeftChanged']),
  TListView: WIN.concat(['OnChange', 'OnChanging', 'OnColumnClick', 'OnCompare', 'OnCustomDraw', 'OnCustomDrawItem', 'OnData', 'OnDeletion', 'OnEdited', 'OnEditing', 'OnInsert', 'OnSelectItem']),
  TTreeView: WIN.concat(['OnChange', 'OnChanging', 'OnCollapsed', 'OnCollapsing', 'OnCompare', 'OnCustomDraw', 'OnDeletion', 'OnEdited', 'OnEditing', 'OnExpanded', 'OnExpanding', 'OnGetImageIndex']),
  TProgressBar: MOUSE.concat(DRAG),
  TStatusBar: MOUSE.concat(DRAG, ['OnDrawPanel', 'OnResize']),
  TDateTimePicker: FOCUS.concat(DRAG, ['OnChange', 'OnClick', 'OnCloseUp', 'OnDropDown', 'OnUserInput']),
  TOpenDialog: ['OnCanClose', 'OnClose', 'OnFolderChange', 'OnSelectionChange', 'OnShow', 'OnTypeChange'],
  TSaveDialog: ['OnCanClose', 'OnClose', 'OnFolderChange', 'OnSelectionChange', 'OnShow', 'OnTypeChange'],
  TPopupMenu: ['OnChange', 'OnPopup'],
  TMainMenu: ['OnChange'],
  TMenuItem: ['OnAdvancedDrawItem', 'OnClick', 'OnDrawItem', 'OnMeasureItem'],
};
/** The TControl ones: a class that is not listed (a custom component such as TMyLedLane). */
const DEFAULT = MOUSE.concat(DRAG);

/** Every event of `cls`, plus `extra` (the ones the .dfm files assign for it), sorted like the Object Inspector. */
function eventsOf(cls, extra) {
  const base = Object.prototype.hasOwnProperty.call(BY, cls) ? BY[cls] : DEFAULT;
  const set = new Set(base);
  for (const e of extra || []) if (/^On[A-Z]\w*$/.test(e)) set.add(e);
  return Array.from(set).sort((a, b) => a.localeCompare(b, 'en'));
}

/** BCB6's name for a new handler: the component's name + the event without "On" (spbSave + OnClick = spbSaveClick); the form's: FormShow. */
function handlerName(component, event, isForm) {
  return (isForm ? 'Form' : String(component || '')) + String(event || '').replace(/^On/, '');
}

/**
 * The default event: what a double-click on the component in the designer opens -- or creates when empty (WPF:
 * "double-click the control ... The default event handler for the control is created"; BCB6 the same: Button1Click,
 * Edit1Change, Timer1Timer, FormCreate).
 */
const CHANGE_DEFAULT = /^T(Edit|MaskEdit|Memo|RichEdit|CSpinEdit|ComboBox|TrackBar|ScrollBar|PageControl|DateTimePicker|TreeView|ListView)$/;
function defaultEventOf(cls, isForm, events) {
  const have = events || eventsOf(isForm ? 'TForm' : cls);
  const want = isForm ? 'OnCreate' : CHANGE_DEFAULT.test(String(cls || '')) ? 'OnChange' : cls === 'TTimer' ? 'OnTimer' : 'OnClick';
  return have.includes(want) ? want : (have[0] || null);
}

/** What the page sends for it (WS form.event takes "click" and "change" only), or null. */
function formEventOf(event) {
  return event === 'OnClick' ? 'click' : event === 'OnChange' ? 'change' : null;
}

/** The handler's parameters (VCL 6 signatures; the port writes them without __fastcall). */
const SIG = {
  OnKeyDown: 'TObject *Sender, WORD &Key, TShiftState Shift',
  OnKeyUp: 'TObject *Sender, WORD &Key, TShiftState Shift',
  OnKeyPress: 'TObject *Sender, char &Key',
  OnMouseDown: 'TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y',
  OnMouseUp: 'TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y',
  OnMouseMove: 'TObject *Sender, TShiftState Shift, int X, int Y',
  OnMouseWheel: 'TObject *Sender, TShiftState Shift, int WheelDelta, TPoint &MousePos, bool &Handled',
  OnMouseWheelDown: 'TObject *Sender, TShiftState Shift, TPoint &MousePos, bool &Handled',
  OnMouseWheelUp: 'TObject *Sender, TShiftState Shift, TPoint &MousePos, bool &Handled',
  OnClose: 'TObject *Sender, TCloseAction &Action',
  OnCloseQuery: 'TObject *Sender, bool &CanClose',
  OnCanResize: 'TObject *Sender, int &NewWidth, int &NewHeight, bool &Resize',
  OnChanging: 'TObject *Sender, bool &AllowChange',
  OnDragDrop: 'TObject *Sender, TObject *Source, int X, int Y',
  OnDragOver: 'TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept',
  OnEndDrag: 'TObject *Sender, TObject *Target, int X, int Y',
  OnStartDrag: 'TObject *Sender, TDragObject *&DragObject',
  OnContextPopup: 'TObject *Sender, TPoint &MousePos, bool &Handled',
  OnDrawCell: 'TObject *Sender, int ACol, int ARow, const TRect &Rect, TGridDrawState State',
  OnSelectCell: 'TObject *Sender, int ACol, int ARow, bool &CanSelect',
  OnSetEditText: 'TObject *Sender, int ACol, int ARow, const AnsiString Value',
  OnGetEditText: 'TObject *Sender, int ACol, int ARow, AnsiString &Value',
  OnDrawItem: 'TWinControl *Control, int Index, const TRect &Rect, TOwnerDrawState State',
  OnMeasureItem: 'TWinControl *Control, int Index, int &Height',
  OnScroll: 'TObject *Sender, TScrollCode ScrollCode, int &ScrollPos',
};
function signatureOf(event) { return SIG[event] || 'TObject *Sender'; }

module.exports = { eventsOf, handlerName, formEventOf, signatureOf, defaultEventOf, BY, DEFAULT };
