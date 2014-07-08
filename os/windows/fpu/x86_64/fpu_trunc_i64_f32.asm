.code
fpu_trunc_i64_f32 proc
  fclex
  fld DWORD PTR [rcx]
  fisttp QWORD PTR [rdx]
  fstsw ax
  ret
fpu_trunc_i64_f32 endp
end

