; ModuleID = 'test13.c'
source_filename = "test13.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @f() #0 {
entry:
  %a = alloca i32, align 4
  %c = alloca i32*, align 8
  %d = alloca i32*, align 8
  %x = alloca i32, align 4
  store i32 1, i32* %a, align 4
  store i32* %a, i32** %c, align 8
  store i32* %a, i32** %d, align 8
  %0 = load i32*, i32** %c, align 8
  store i32 0, i32* %0, align 4
  %1 = load i32*, i32** %d, align 8
  %2 = load i32, i32* %1, align 4
  %div = sdiv i32 1, %2
  store i32 %div, i32* %x, align 4
  %3 = load i32, i32* %x, align 4
  ret i32 %3
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.1- (branches/release_80)"}
