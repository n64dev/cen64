.code
fpu_round_i32_f64 proc
  fclex
  fld QWORD PTR [rcx]
  fistp DWORD PTR [rdx]
  fstsw ax
  ret
fpu_round_i32_f64 endp
end

