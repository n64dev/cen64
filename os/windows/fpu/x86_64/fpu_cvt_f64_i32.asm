.code
fpu_cvt_f64_i32 proc
  fclex
  fild DWORD PTR [rcx]
  fstp QWORD PTR [rdx]
  fstsw ax
  ret
fpu_cvt_f64_i32 endp
end

