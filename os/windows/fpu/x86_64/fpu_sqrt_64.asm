.code
fpu_sqrt_64 proc
  fclex
  fld QWORD PTR [rcx]
  fsqrt
  fstp QWORD PTR [r8]
  fstsw ax
  ret
fpu_sqrt_64 endp
end

