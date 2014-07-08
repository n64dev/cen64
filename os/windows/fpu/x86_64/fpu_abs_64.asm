.code
fpu_abs_64 proc
  fclex
  fld QWORD PTR [rcx]
  fabs
  fstp QWORD PTR [r8]
  fstsw ax
  ret
fpu_abs_64 endp
end

