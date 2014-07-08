.code
fpu_neg_64 proc
  fclex
  fld QWORD PTR [rcx]
  fchs
  fstp QWORD PTR [r8]
  fstsw ax
  ret
fpu_neg_64 endp
end

