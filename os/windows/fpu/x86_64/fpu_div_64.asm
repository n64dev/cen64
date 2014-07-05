.code
fpu_div_64 proc
  fclex
  fld QWORD PTR [rcx]
  fld QWORD PTR [rdx]
  fdivrp
  fstp QWORD PTR [r8]
  fstsw ax
  ret
fpu_div_64 endp
end

