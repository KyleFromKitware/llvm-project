// RUN: grep -Ev "// *[A-Z-]+:" %s > %t.cpp
// RUN: clang-tidy %t.cpp -checks='-*,modernize-use-trailing-return-type,readability-identifier-length' -fix -fix-mode=nolint -nolint-prefix='FIXME: ' -export-fixes=%t.yaml -- > %t.msg 2>&1
// RUN: FileCheck -input-file=%t.cpp %s
// RUN: FileCheck -input-file=%t.msg -check-prefix=CHECK-MESSAGES %s
// RUN: FileCheck -input-file=%t.yaml -check-prefix=CHECK-YAML %s

// CHECK: /* FIXME: NOLINTNEXTLINE(modernize-use-trailing-return-type) */
// CHECK-NEXT: int func() {
// CHECK-MESSAGES: note: FIX-IT added NOLINT to suppress warning
// CHECK-YAML: ReplacementText: "/* FIXME: NOLINTNEXTLINE(modernize-use-trailing-return-type) */\n"
int func() {
  // CHECK: /* FIXME: NOLINTNEXTLINE(readability-identifier-length) */
  // CHECK-NEXT: int i = 0;
  // CHECK-MESSAGES: note: FIX-IT added NOLINT to suppress warning
  // CHECK-YAML: ReplacementText: "  /* FIXME: NOLINTNEXTLINE(readability-identifier-length) */\n"
  int i = 0;
  return i;
}
