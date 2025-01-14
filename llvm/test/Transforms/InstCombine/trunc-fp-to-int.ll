; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt -passes=instcombine -S -o - %s | FileCheck %s

; Tests if an integer type can cover the entire range of an input
; FP value. If so, we can remove an intermediate cast to a smaller
; int type (remove a truncate).

define i16 @half_fptoui_i17_i16(half %x) {
; CHECK-LABEL: @half_fptoui_i17_i16(
; CHECK-NEXT:    [[I:%.*]] = fptoui half [[X:%.*]] to i17
; CHECK-NEXT:    [[R:%.*]] = trunc i17 [[I]] to i16
; CHECK-NEXT:    ret i16 [[R]]
;
  %i = fptoui half %x to i17
  %r = trunc i17 %i to i16
  ret i16 %r
}

; Negative test - not enough bits to hold max half value (65504).

define i15 @half_fptoui_i17_i15(half %x) {
; CHECK-LABEL: @half_fptoui_i17_i15(
; CHECK-NEXT:    [[I:%.*]] = fptoui half [[X:%.*]] to i17
; CHECK-NEXT:    [[R:%.*]] = trunc i17 [[I]] to i15
; CHECK-NEXT:    ret i15 [[R]]
;
  %i = fptoui half %x to i17
  %r = trunc i17 %i to i15
  ret i15 %r
}

; Wider intermediate type is ok.

define i16 @half_fptoui_i32_i16(half %x) {
; CHECK-LABEL: @half_fptoui_i32_i16(
; CHECK-NEXT:    [[I:%.*]] = fptoui half [[X:%.*]] to i32
; CHECK-NEXT:    [[R:%.*]] = trunc i32 [[I]] to i16
; CHECK-NEXT:    ret i16 [[R]]
;
  %i = fptoui half %x to i32
  %r = trunc i32 %i to i16
  ret i16 %r
}

; Wider final type is ok.
; TODO: Handle non-simple result type.

define i17 @half_fptoui_i32_i17(half %x) {
; CHECK-LABEL: @half_fptoui_i32_i17(
; CHECK-NEXT:    [[I:%.*]] = fptoui half [[X:%.*]] to i32
; CHECK-NEXT:    [[R:%.*]] = trunc i32 [[I]] to i17
; CHECK-NEXT:    ret i17 [[R]]
;
  %i = fptoui half %x to i32
  %r = trunc i32 %i to i17
  ret i17 %r
}

; Vectors work too.

define <4 x i16> @half_fptoui_4xi32_4xi16(<4 x half> %x) {
; CHECK-LABEL: @half_fptoui_4xi32_4xi16(
; CHECK-NEXT:    [[I:%.*]] = fptoui <4 x half> [[X:%.*]] to <4 x i32>
; CHECK-NEXT:    [[R:%.*]] = trunc <4 x i32> [[I]] to <4 x i16>
; CHECK-NEXT:    ret <4 x i16> [[R]]
;
  %i = fptoui <4 x half> %x to <4 x i32>
  %r = trunc <4 x i32> %i to <4 x i16>
  ret <4 x i16> %r
}

define i128 @bfloat_fptoui_i129_i128(bfloat %x) {
; CHECK-LABEL: @bfloat_fptoui_i129_i128(
; CHECK-NEXT:    [[I:%.*]] = fptoui bfloat [[X:%.*]] to i129
; CHECK-NEXT:    [[R:%.*]] = trunc i129 [[I]] to i128
; CHECK-NEXT:    ret i128 [[R]]
;
  %i = fptoui bfloat %x to i129
  %r = trunc i129 %i to i128
  ret i128 %r
}

; Negative test - not enough bits to hold max bfloat value (2**127 * (2 − 2**−7))

define i127 @bfloat_fptoui_i128_i127(bfloat %x) {
; CHECK-LABEL: @bfloat_fptoui_i128_i127(
; CHECK-NEXT:    [[I:%.*]] = fptoui bfloat [[X:%.*]] to i128
; CHECK-NEXT:    [[R:%.*]] = trunc i128 [[I]] to i127
; CHECK-NEXT:    ret i127 [[R]]
;
  %i = fptoui bfloat %x to i128
  %r = trunc i128 %i to i127
  ret i127 %r
}

define i128 @float_fptoui_i129_i128(float %x) {
; CHECK-LABEL: @float_fptoui_i129_i128(
; CHECK-NEXT:    [[I:%.*]] = fptoui float [[X:%.*]] to i129
; CHECK-NEXT:    [[R:%.*]] = trunc i129 [[I]] to i128
; CHECK-NEXT:    ret i128 [[R]]
;
  %i = fptoui float %x to i129
  %r = trunc i129 %i to i128
  ret i128 %r
}

; TODO: We could transform with multiple users.
declare void @use(i129)
define i128 @float_fptoui_i129_i128_use(float %x) {
; CHECK-LABEL: @float_fptoui_i129_i128_use(
; CHECK-NEXT:    [[I:%.*]] = fptoui float [[X:%.*]] to i129
; CHECK-NEXT:    call void @use(i129 [[I]])
; CHECK-NEXT:    [[R:%.*]] = trunc i129 [[I]] to i128
; CHECK-NEXT:    ret i128 [[R]]
;
  %i = fptoui float %x to i129
  call void @use(i129 %i)
  %r = trunc i129 %i to i128
  ret i128 %r
}

; Negative test - not enough bits to hold max float value (2**127 * (2 − 2**−23))

define i127 @float_fptoui_i128_i127(float %x) {
; CHECK-LABEL: @float_fptoui_i128_i127(
; CHECK-NEXT:    [[I:%.*]] = fptoui float [[X:%.*]] to i128
; CHECK-NEXT:    [[R:%.*]] = trunc i128 [[I]] to i127
; CHECK-NEXT:    ret i127 [[R]]
;
  %i = fptoui float %x to i128
  %r = trunc i128 %i to i127
  ret i127 %r
}

define i1024 @double_fptoui_i1025_i1024(double %x) {
; CHECK-LABEL: @double_fptoui_i1025_i1024(
; CHECK-NEXT:    [[I:%.*]] = fptoui double [[X:%.*]] to i1025
; CHECK-NEXT:    [[R:%.*]] = trunc i1025 [[I]] to i1024
; CHECK-NEXT:    ret i1024 [[R]]
;
  %i = fptoui double %x to i1025
  %r = trunc i1025 %i to i1024
  ret i1024 %r
}

; Negative test - not enough bits to hold max double value (2**1023 * (2 − 2**−52))

define i1023 @double_fptoui_i1024_i1023(double %x) {
; CHECK-LABEL: @double_fptoui_i1024_i1023(
; CHECK-NEXT:    [[I:%.*]] = fptoui double [[X:%.*]] to i1024
; CHECK-NEXT:    [[R:%.*]] = trunc i1024 [[I]] to i1023
; CHECK-NEXT:    ret i1023 [[R]]
;
  %i = fptoui double %x to i1024
  %r = trunc i1024 %i to i1023
  ret i1023 %r
}

; Negative test - not enough bits to hold min half value (-65504).

define i16 @half_fptosi_i17_i16(half %x) {
; CHECK-LABEL: @half_fptosi_i17_i16(
; CHECK-NEXT:    [[I:%.*]] = fptosi half [[X:%.*]] to i17
; CHECK-NEXT:    [[R:%.*]] = trunc i17 [[I]] to i16
; CHECK-NEXT:    ret i16 [[R]]
;
  %i = fptosi half %x to i17
  %r = trunc i17 %i to i16
  ret i16 %r
}

define i17 @half_fptosi_i18_i17(half %x) {
; CHECK-LABEL: @half_fptosi_i18_i17(
; CHECK-NEXT:    [[I:%.*]] = fptosi half [[X:%.*]] to i18
; CHECK-NEXT:    [[R:%.*]] = trunc i18 [[I]] to i17
; CHECK-NEXT:    ret i17 [[R]]
;
  %i = fptosi half %x to i18
  %r = trunc i18 %i to i17
  ret i17 %r
}

; Wider intermediate type is ok.
; TODO: Handle non-simple result type.

define i17 @half_fptosi_i32_i17(half %x) {
; CHECK-LABEL: @half_fptosi_i32_i17(
; CHECK-NEXT:    [[I:%.*]] = fptosi half [[X:%.*]] to i32
; CHECK-NEXT:    [[R:%.*]] = trunc i32 [[I]] to i17
; CHECK-NEXT:    ret i17 [[R]]
;
  %i = fptosi half %x to i32
  %r = trunc i32 %i to i17
  ret i17 %r
}

; Wider final type is ok.
; TODO: Handle non-simple result type.

define i18 @half_fptosi_i32_i18(half %x) {
; CHECK-LABEL: @half_fptosi_i32_i18(
; CHECK-NEXT:    [[I:%.*]] = fptosi half [[X:%.*]] to i32
; CHECK-NEXT:    [[R:%.*]] = trunc i32 [[I]] to i18
; CHECK-NEXT:    ret i18 [[R]]
;
  %i = fptosi half %x to i32
  %r = trunc i32 %i to i18
  ret i18 %r
}

; Vectors work too.

define <4 x i17> @half_fptosi_4xi32_4xi17(<4 x half> %x) {
; CHECK-LABEL: @half_fptosi_4xi32_4xi17(
; CHECK-NEXT:    [[I:%.*]] = fptosi <4 x half> [[X:%.*]] to <4 x i32>
; CHECK-NEXT:    [[R:%.*]] = trunc <4 x i32> [[I]] to <4 x i17>
; CHECK-NEXT:    ret <4 x i17> [[R]]
;
  %i = fptosi <4 x half> %x to <4 x i32>
  %r = trunc <4 x i32> %i to <4 x i17>
  ret <4 x i17> %r
}

; Negative test - not enough bits to hold min float value.

define i128 @bfloat_fptosi_i129_i128(bfloat %x) {
; CHECK-LABEL: @bfloat_fptosi_i129_i128(
; CHECK-NEXT:    [[I:%.*]] = fptosi bfloat [[X:%.*]] to i129
; CHECK-NEXT:    [[R:%.*]] = trunc i129 [[I]] to i128
; CHECK-NEXT:    ret i128 [[R]]
;
  %i = fptosi bfloat %x to i129
  %r = trunc i129 %i to i128
  ret i128 %r
}

define i129 @bfloat_fptosi_i130_i129(bfloat %x) {
; CHECK-LABEL: @bfloat_fptosi_i130_i129(
; CHECK-NEXT:    [[I:%.*]] = fptosi bfloat [[X:%.*]] to i130
; CHECK-NEXT:    [[R:%.*]] = trunc i130 [[I]] to i129
; CHECK-NEXT:    ret i129 [[R]]
;
  %i = fptosi bfloat %x to i130
  %r = trunc i130 %i to i129
  ret i129 %r
}

define i129 @float_fptosi_i130_i129(float %x) {
; CHECK-LABEL: @float_fptosi_i130_i129(
; CHECK-NEXT:    [[I:%.*]] = fptosi float [[X:%.*]] to i130
; CHECK-NEXT:    [[R:%.*]] = trunc i130 [[I]] to i129
; CHECK-NEXT:    ret i129 [[R]]
;
  %i = fptosi float %x to i130
  %r = trunc i130 %i to i129
  ret i129 %r
}

; Negative test - not enough bits to hold min float value.

define i128 @float_fptosi_i129_i128(float %x) {
; CHECK-LABEL: @float_fptosi_i129_i128(
; CHECK-NEXT:    [[I:%.*]] = fptosi float [[X:%.*]] to i129
; CHECK-NEXT:    [[R:%.*]] = trunc i129 [[I]] to i128
; CHECK-NEXT:    ret i128 [[R]]
;
  %i = fptosi float %x to i129
  %r = trunc i129 %i to i128
  ret i128 %r
}

define i1025 @double_fptosi_i1026_i1025(double %x) {
; CHECK-LABEL: @double_fptosi_i1026_i1025(
; CHECK-NEXT:    [[I:%.*]] = fptosi double [[X:%.*]] to i1026
; CHECK-NEXT:    [[R:%.*]] = trunc i1026 [[I]] to i1025
; CHECK-NEXT:    ret i1025 [[R]]
;
  %i = fptosi double %x to i1026
  %r = trunc i1026 %i to i1025
  ret i1025 %r
}

; Negative test - not enough bits to hold min double value.

define i1024 @double_fptosi_i1025_i1024(double %x) {
; CHECK-LABEL: @double_fptosi_i1025_i1024(
; CHECK-NEXT:    [[I:%.*]] = fptosi double [[X:%.*]] to i1025
; CHECK-NEXT:    [[R:%.*]] = trunc i1025 [[I]] to i1024
; CHECK-NEXT:    ret i1024 [[R]]
;
  %i = fptosi double %x to i1025
  %r = trunc i1025 %i to i1024
  ret i1024 %r
}
