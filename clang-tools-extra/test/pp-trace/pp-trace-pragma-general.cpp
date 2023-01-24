// RUN: pp-trace -callbacks '*,-FileChanged,-MacroDefined' -extra-arg-before=-xc++-header %s -- | FileCheck --strict-whitespace %s

#pragma clang diagnostic push
#pragma clang diagnostic pop
#pragma clang diagnostic ignored "-Wformat"
#pragma clang diagnostic warning "-Wformat"
#pragma clang diagnostic error "-Wformat"
#pragma clang diagnostic fatal "-Wformat"

#pragma GCC diagnostic push
#pragma GCC diagnostic pop
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic warning "-Wformat"
#pragma GCC diagnostic error "-Wformat"
#pragma GCC diagnostic fatal "-Wformat"

void foo() {
#pragma clang __debug captured
{ }
}

#pragma once

// CHECK: ---
// CHECK-NEXT: - Callback: PragmaDirective
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:3:1", Kind: PIK_HashPragma}
// CHECK-NEXT: - Callback: PragmaDiagnosticPush
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:3:1", Kind: PIK_HashPragma}
// CHECK-NEXT:   Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:3:15"
// CHECK-NEXT:   Namespace: clang
// CHECK-NEXT: - Callback: PragmaDirective
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:4:1", Kind: PIK_HashPragma}
// CHECK-NEXT: - Callback: PragmaDiagnosticPop
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:4:1", Kind: PIK_HashPragma}
// CHECK-NEXT:   Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:4:15"
// CHECK-NEXT:   Namespace: clang
// CHECK-NEXT: - Callback: PragmaDirective
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:5:1", Kind: PIK_HashPragma}
// CHECK-NEXT: - Callback: PragmaDiagnostic
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:5:1", Kind: PIK_HashPragma}
// CHECK-NEXT:   Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:5:15"
// CHECK-NEXT:   Namespace: clang
// CHECK-NEXT:   Mapping: MAP_IGNORE
// CHECK-NEXT:   Str: -Wformat
// CHECK-NEXT: - Callback: PragmaDirective
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:6:1", Kind: PIK_HashPragma}
// CHECK-NEXT: - Callback: PragmaDiagnostic
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:6:1", Kind: PIK_HashPragma}
// CHECK-NEXT:   Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:6:15"
// CHECK-NEXT:   Namespace: clang
// CHECK-NEXT:   Mapping: MAP_WARNING
// CHECK-NEXT:   Str: -Wformat
// CHECK-NEXT: - Callback: PragmaDirective
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:7:1", Kind: PIK_HashPragma}
// CHECK-NEXT: - Callback: PragmaDiagnostic
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:7:1", Kind: PIK_HashPragma}
// CHECK-NEXT:   Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:7:15"
// CHECK-NEXT:   Namespace: clang
// CHECK-NEXT:   Mapping: MAP_ERROR
// CHECK-NEXT:   Str: -Wformat
// CHECK-NEXT: - Callback: PragmaDirective
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:8:1", Kind: PIK_HashPragma}
// CHECK-NEXT: - Callback: PragmaDiagnostic
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:8:1", Kind: PIK_HashPragma}
// CHECK-NEXT:   Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:8:15"
// CHECK-NEXT:   Namespace: clang
// CHECK-NEXT:   Mapping: MAP_FATAL
// CHECK-NEXT:   Str: -Wformat
// CHECK-NEXT: - Callback: PragmaDirective
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:10:1", Kind: PIK_HashPragma}
// CHECK-NEXT: - Callback: PragmaDiagnosticPush
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:10:1", Kind: PIK_HashPragma}
// CHECK-NEXT:   Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:10:13"
// CHECK-NEXT:   Namespace: GCC
// CHECK-NEXT: - Callback: PragmaDirective
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:11:1", Kind: PIK_HashPragma}
// CHECK-NEXT: - Callback: PragmaDiagnosticPop
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:11:1", Kind: PIK_HashPragma}
// CHECK-NEXT:   Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:11:13"
// CHECK-NEXT:   Namespace: GCC
// CHECK-NEXT: - Callback: PragmaDirective
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:12:1", Kind: PIK_HashPragma}
// CHECK-NEXT: - Callback: PragmaDiagnostic
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:12:1", Kind: PIK_HashPragma}
// CHECK-NEXT:   Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:12:13"
// CHECK-NEXT:   Namespace: GCC
// CHECK-NEXT:   Mapping: MAP_IGNORE
// CHECK-NEXT:   Str: -Wformat
// CHECK-NEXT: - Callback: PragmaDirective
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:13:1", Kind: PIK_HashPragma}
// CHECK-NEXT: - Callback: PragmaDiagnostic
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:13:1", Kind: PIK_HashPragma}
// CHECK-NEXT:   Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:13:13"
// CHECK-NEXT:   Namespace: GCC
// CHECK-NEXT:   Mapping: MAP_WARNING
// CHECK-NEXT:   Str: -Wformat
// CHECK-NEXT: - Callback: PragmaDirective
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:14:1", Kind: PIK_HashPragma}
// CHECK-NEXT: - Callback: PragmaDiagnostic
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:14:1", Kind: PIK_HashPragma}
// CHECK-NEXT:   Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:14:13"
// CHECK-NEXT:   Namespace: GCC
// CHECK-NEXT:   Mapping: MAP_ERROR
// CHECK-NEXT:   Str: -Wformat
// CHECK-NEXT: - Callback: PragmaDirective
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:15:1", Kind: PIK_HashPragma}
// CHECK-NEXT: - Callback: PragmaDiagnostic
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:15:1", Kind: PIK_HashPragma}
// CHECK-NEXT:   Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:15:13"
// CHECK-NEXT:   Namespace: GCC
// CHECK-NEXT:   Mapping: MAP_FATAL
// CHECK-NEXT:   Str: -Wformat
// CHECK-NEXT: - Callback: PragmaDirective
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:18:1", Kind: PIK_HashPragma}
// CHECK-NEXT: - Callback: PragmaDebug
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:18:1", Kind: PIK_HashPragma}
// CHECK-NEXT:   Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:18:23"
// CHECK-NEXT:   DebugType: captured
// CHECK-NEXT: - Callback: PragmaDirective
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:22:1", Kind: PIK_HashPragma}
// CHECK-NEXT: - Callback: PragmaOnce
// CHECK-NEXT:   Introducer: {Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:22:1", Kind: PIK_HashPragma}
// CHECK-NEXT:   Loc: "{{.*}}{{[/\\]}}pp-trace-pragma-general.cpp:22:9"
// CHECK-NEXT: - Callback: EndOfMainFile
// CHECK-NEXT: ...
