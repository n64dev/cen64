.code
fpu_mul_64 proc
  fclex
  fld QWORD PTR [rcx]
  fld QWORD PTR [rdx]
  fmulp
  fstp QWORD PTR [r8]
  fstsw ax
  ret
fpu_mul_64 endp
end

