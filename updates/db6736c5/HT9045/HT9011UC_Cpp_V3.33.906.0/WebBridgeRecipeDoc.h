// =============================================================================
//  WebBridgeRecipeDoc.h -- recipe/settings documents -> the browser's JSON shape.
//
//  AI(W906-FW-C1R) 20260911. User ruling 20260911: the browser must be able to
//  read AND write the machine's recipe documents, targeting the real shared
//  files so they can be cross-checked in BCB6.
//
//  Read half: RecipeDocToJson.  Write half: RecipeDocApplyEdits (C1-W, below).
//
//  =============================================================================
//  THE WRITE IS A SURGICAL TEXT EDIT, NOT A RE-SERIALISATION
//  =============================================================================
//  The obvious write is TMemIniFile + WriteString + UpdateFile. It is wrong
//  here, and measurably so. UpdateFile calls store_.SaveToFile
//  (IniFiles.cpp:277-279), which rebuilds the file FROM THE PARSED STORE. That
//  discards everything the store did not model:
//    * comment lines and blank-line layout;
//    * the original key order and indentation;
//    * every line the grammar dropped -- and on
//      IniData/Data/SNH-FCBGA3757-68X68-H80C/ArmCondition.Data that is 23 real
//      parameters sitting in duplicate [All] and [Iput  Arm] blocks that the
//      store discards on load. Re-serialising that file would turn "the machine
//      ignores these" into "these are gone".
//  So RecipeDocApplyEdits rewrites ONLY the value text after the first '=' on
//  the lines it was asked to change, and copies every other byte through
//  untouched. That is what makes `raw` meaningful: a field nobody edited is
//  byte-identical afterwards, not merely equivalent.
//
//  =============================================================================
//  WHY THIS IS NOT IN WebBridge/
//  =============================================================================
//  JsonWriter.h's layer rule: "WebBridge/ must stay independent of VCL /
//  vclcompat." This TU deliberately uses vclcompat's TIniFile, so it lives at
//  the tree root alongside WebBridgeTags.cpp -- the sibling that is already
//  allowed to mix the two worlds.
//
//  Using vclcompat's TIniStore (TIniFile until A5 20260924) rather than a fresh parser is the whole point. Its grammar is
//  documented as BCB6-faithful (vclcompat/IniFiles.cpp:158-171) and differs from
//  the obvious implementation in ways that would have produced wrong answers:
//    * the FIRST '=' splits, not the last;
//    * the key is trimmed but THE VALUE IS NOT -- BCB6 preserves trailing and
//      internal spaces, and the browser's `raw` field exists precisely so a
//      value round-trips byte-for-byte;
//    * keys appearing before any [section] are DROPPED (no global section);
//    * '[name]' may omit its trailing ']';
//    * ';' and '#' comment lines are skipped; CR, LF and CRLF all terminate.
//  [AI(W906-A5-INIREAD) 20260924 MEASURED vs kernel32: BCB6 TIniFile = GetPrivateProfileStringA DOES trim + unquote values and keeps '#' lines as keys, so `raw` is verbatim for the ROUND-TRIP, not "what BCB6 TIniFile reads".] A hand-rolled reader that trimmed values would disagree with what BCB6 reads
//  from the same file, which is exactly the comparison the user asked for.
//
//  =============================================================================
//  ⚠ TIniStore (TIniFile until A5 20260924), NEVER TMemIniFile
//  =============================================================================
//  TMemIniFile's destructor calls flush() UNCONDITIONALLY
//  (vclcompat/IniFiles.cpp:370-375 -> store_.SaveToFile(FileName)). Constructing
//  one over a recipe file and letting it fall out of scope REWRITES THAT FILE --
//  with zero mutations -- reordering keys and dropping comments. That is not a
//  hypothetical in this tree: system\Gerneral.ini was rewritten wholesale on
//  20260817 by a tool that opened a real file in the wrong mode.
//
//  TIniFile's destructor is a genuine no-op (IniFiles.cpp:272-275) because it is
//  write-through: it writes on MUTATION, and this file calls no Write* method.
//  So the read path cannot write, by construction rather than by discipline.
//
//  It also means the reader does NOT go through the golden load path. Both of
//  cContact's loaders are gated as writers for good reason (forms/fContact.h
//  :257-262): ReadFile is "full of CheckAndReadIniData (write-back)", which
//  reaches common.cpp's ReadWriteIni -> CheckAndReadIniData and writes defaults
//  back into config.ini from what reads like a read. The browser asking to SEE
//  a recipe must not mutate it.
//
//  =============================================================================
//  THE OUTPUT SHAPE IS NOT OURS TO DESIGN
//  =============================================================================
//  It is the web author's, already published in
//  JSON/Production-update.json -> state.context.physicalRecipe.documents.<doc>,
//  and measured across all 16 documents there (16,551 fields):
//
//    { "path": "D:\\HT9045\\IniData\\Data\\<recipe>\\Contact.Data",
//      "available": true,
//      "sections": {
//        "Test Arm1": {
//          "Pick Up": { "value": 24.11, "type": "float", "raw": "24.11" },
//          "Up Speed": { "value": 1,    "type": "int",   "raw": "1" },
//          "EPControl": { "value": "",  "type": "string", "raw": "" } } } }
//
//  Document-level keys are exactly `path`, `available`, `sections` -- no more.
//  `type` is exactly one of "int" (15,210 observed), "float" (963), "string"
//  (378, and an EMPTY value is a string, not null).
//
//  `raw` is the load-bearing field: it preserves the original text, so   [AI(W906-RECIPE-BCB-R13) 20260926: every cell also carries "bcb" = what BCB6 TIniFile reads (trim 0x01..0x20, drop one matching quote pair, cut to 2047; WebBridgeRecipeBcb.h). The browser SHOWS bcb and value/type are derived from it; only raw is ever written back (an untouched field is sent back as raw). Known gap: a section header with 2+ "]" -- store takes the last, kernel32 the first.]
//  "23.6100" survives a round trip that `value` alone (23.61) would silently
//  reformat. The write half must write `raw` back verbatim for every field the
//  operator did not touch.
// =============================================================================
#ifndef HT9045_WEBBRIDGE_RECIPE_DOC_H
#define HT9045_WEBBRIDGE_RECIPE_DOC_H

#include <string>
#include <vector>

#include "vclcompat/AnsiString.h"

namespace ht9045 {

// How a raw INI value is classified for the browser contract. Derived from the
// value's TRIMMED text, mirroring BCB6's StrToIntDef / ReadFloat semantics
// (vclcompat/IniFiles.cpp:20-62), while `raw` keeps the untrimmed original.
enum RecipeFieldType {
    kRecipeFieldInt,      // whole trimmed text is an integer      -> "int"
    kRecipeFieldFloat,    // whole trimmed text is a decimal       -> "float"
    kRecipeFieldString    // anything else, INCLUDING empty        -> "string"
};

// Exposed for tests: the classifier, on the raw (untrimmed) value text.
RecipeFieldType ClassifyRecipeField(const std::string& rawVerbatim);

// Serialise one recipe/settings document as the JSON object described above.
//
// READ-ONLY. Opens `path` for reading through TIniFile and calls no Write*
// method, so the file is not touched. A missing or unreadable file is not an
// error: it yields `"available": false` with an empty `sections`, which is what
// the web author's own documents do for an absent file.
//
// Returns a complete JSON object. Strings are emitted through
// WebBridge/JsonWriter, so a legacy Big5 byte sequence is transcoded to UTF-8
// rather than dropping the browser's whole frame. (Measured 20260911: all 1,012
// .Data files across 63 recipe directories are pure ASCII today, so this path
// is currently exercised only by the ASCII fast path -- but a new recipe from a
// customer with Chinese text in a value would otherwise break every screen.)
std::string RecipeDocToJson(const vclcompat::AnsiString& path);

// ---------------------------------------------------------------------------
//  C1-W -- the write half.
//
//  Takes EDITS, not JSON. Parsing the browser's frame belongs at the command
//  dispatch boundary, which already owns cJSON; keeping it out of here is what
//  lets this TU link against vclcompat + ht9045_webbridge and nothing else, and
//  lets its test run without the god-stack.
// ---------------------------------------------------------------------------
struct RecipeFieldEdit {
    std::string section;
    std::string key;
    std::string rawValue;   // written VERBATIM after the first '=' -- the caller
                            // owns formatting, because only the caller knows how
                            // many decimals the original had.
};

enum RecipeWriteMode {
    // Default. Reports exactly what would change and writes NOTHING -- not the
    // target, not a backup, not a temp file. `wb_publish`/`wb_serve` are
    // default-dry for the same reason and this follows them.
    kRecipeWriteDryRun,
    // Backup, write a temp file, atomic replace. Touches the real machine file.
    kRecipeWriteApply
};

struct RecipeWriteResult {
    bool        ok;
    int         changed;      // edits whose text differs from the file
    int         identical;    // edits already equal to the file -- not rewritten
    int         notFound;     // edits naming a (section,key) the file has no line for
    std::string error;        // empty iff ok
    std::string backupPath;   // written only in kRecipeWriteApply
};

// Apply `edits` to `path`.
//
// ⚠ THE HANDLER'S GATES ARE NOT IN THIS PATH. golden spbSaveClick
// (cContact.cpp:14179 in V912) refuses the save outright when
// IniConfig.bA02DisableSaveParsWhenSwitchToOp and AccessLevel==0, bails out in
// K-Temperature mode, warns on a changed DOUBLE_EP diameter, fires
// EventReport(SECS_EVENT.SaveRecipe), and runs a CC_KYEC_LEE distance check.
// None of that happens here -- this function writes a file. Production use has
// to reach those checks by calling the translated handler
// (forms/fContact.h GATE (W-03)/(W-04), still gated), NOT by this layer
// re-implementing them: WEBBRIDGE_WRITEPATH_DESIGN.md section 1 is explicit
// that the web layer must not stack its own interlocks.
//
// Guarantees in kRecipeWriteApply:
//   * a backup is written FIRST and its path returned; if the backup fails,
//     nothing else is attempted;
//   * the new content goes to a temp file, is closed, and only then replaces the
//     target -- so a reader can never see a half-written document (the web
//     author's own Runtime-bridge-contract.json specifies exactly these writer
//     steps for the C++ side);
//   * every byte not on an edited line is copied through unchanged;
//   * an edit naming a missing (section,key) is COUNTED AND REPORTED, never
//     appended. Appending would invent a key in a file whose grammar the
//     machine reads first-wins, and a key added in the wrong block is a key the
//     machine silently ignores.
//   * SetMD5ByFolder is deliberately NOT called: golden's own
//     TfContact::SaveSetupFile does not call it either (measured 20260911), and
//     matching golden is the point. The folder checksum therefore goes stale on
//     a write exactly as it does today -- see the note in the .cpp.
RecipeWriteResult RecipeDocApplyEdits(const vclcompat::AnsiString& path,
                                      const std::vector<RecipeFieldEdit>& edits,
                                      RecipeWriteMode mode);

} // namespace ht9045

#endif // HT9045_WEBBRIDGE_RECIPE_DOC_H
