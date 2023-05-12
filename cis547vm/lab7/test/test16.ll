; ModuleID = 'test16.c'
source_filename = "test16.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @f(i32 %arg) #0 {
entry:
  %arg.addr = alloca i32, align 4
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %x = alloca i32*, align 8
  %y = alloca i32*, align 8
  %z = alloca i32*, align 8
  %z2 = alloca i32, align 4
  store i32 %arg, i32* %arg.addr, align 4
  store i32 1, i32* %a, align 4
  store i32 1, i32* %b, align 4
  %0 = load i32, i32* %arg.addr, align 4
  %tobool = icmp ne i32 %0, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store i32* %a, i32** %x, align 8
  store i32* %b, i32** %y, align 8
  br label %if.end

if.else:                                          ; preds = %entry
  store i32* %b, i32** %x, align 8
  store i32* %a, i32** %y, align 8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %1 = load i32, i32* %arg.addr, align 4
  %tobool1 = icmp ne i32 %1, 0
  br i1 %tobool1, label %if.then2, label %if.else3

if.then2:                                         ; preds = %if.end
  %2 = load i32*, i32** %x, align 8
  store i32* %2, i32** %z, align 8
  br label %if.end4

if.else3:                                         ; preds = %if.end
  %3 = load i32*, i32** %y, align 8
  store i32* %3, i32** %z, align 8
  br label %if.end4

if.end4:                                          ; preds = %if.else3, %if.then2
  %4 = load i32*, i32** %z, align 8
  %5 = load i32, i32* %4, align 4
  %div = sdiv i32 1, %5
  store i32 %div, i32* %z2, align 4
  ret i32 0
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.1- (branches/release_80)"}
