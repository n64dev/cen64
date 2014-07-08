.code
fpu_round_i64_f32 proc
  fclex
  fld DWORD PTR [rcx]
  fistp QWORD PTR [rdx]
  fstsw ax
  ret
fpu_round_i64_f32 endp
end

