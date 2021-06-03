$size=5;

$prefix = "mat${size}_";
$type   = "${prefix}t";

@axis = ();
 
if ($size < 5) {
	@axis = ("x", "y", "z", "t");
} else {
	for ($i = 0; $i < $size; $i++) {
		push(@axis, "x$i");
	}
}

#
# Create header file:
open (H, ">matrix${size}.h");
print H <<EOB
#ifndef _matrix${size}_h_
#define _matrix${size}_h_

#ifdef __cplusplus 
extern "C" {
#endif

#include <stdio.h>
#include <math.h>
#define trig_sin(x)  sin(x)
#define trig_cos(x)  cos(x)

#define matSIZE ${size}

#define ${prefix}dump(A, str) ${prefix}fdump(NULL, (A), (str))

typedef struct {
	double a[matSIZE];
} vect${size}_t, *pvect${size}_t;

typedef struct {
	double a[matSIZE][matSIZE]; 
} ${type}, *p${type};

typedef enum {
EOB
;

for ($i = 0; $i < $size; $i++) {
	print H "\t_" . $axis[$i] . "_ = $i,\n";
}

print H <<EOB
} axis_t;

${type} ${prefix}null();
${type} ${prefix}ident();

void     ${prefix}fdump(FILE *f, ${type} A, char *name);
${type}   ${prefix}add(${type} A, ${type} B);
${type}   ${prefix}sub(${type} A, ${type} B);
${type}   ${prefix}mul(${type} A, ${type} B);
vect${size}_t  ${prefix}mul_vect${size}(${type} M, vect${size}_t v);
${type}   ${type}ranspose(${type} A);
double   ${prefix}det(${type} A);
${type}   ${prefix}scale(double sx, double sy);
${type}   ${prefix}rotation(double angle);
${type}   ${type}ranslation(double dx, double dy);
${type}   ${prefix}projection(double k, double l, double m);

#ifdef __cplusplus 
}
#endif
#endif
EOB
;
close(H);

#
# Create the C file: 
open (C, ">matrix${size}.c");
print C <<EOB
#include <stdlib.h>
#include <stdio.h>

#include "matrix${size}.h"

EOB
;
print C "static ${type} __${prefix}null = {{";

for ($i = 0; $i < $size; $i++) {
	if ($i > 0) {
		print C ",";
	}
	print C "{";
	for ($j = 0; $j < $size; $j++) {
		if ($j > 0) {
			print C ",";
		}
		print C "0"
	} 
	print C "}";
}
print C "}};\n";

print C "static ${type} __${prefix}ident = {{";
for ($i = 0; $i < $size; $i++) {
	if ($i > 0) {
		print C ",";
	}
	print C "{";
	for ($j = 0; $j < $size; $j++) {
		if ($j > 0) {
			print C ",";
		}
		print C "0"
	} 
	print C "}";
}
print C "}};\n\n";
print C "${prefix}t ${prefix}null()  { return __${prefix}null;  }\n";
print C "${prefix}t ${prefix}ident() { return __${prefix}ident; }\n\n";

print C <<EOB
void
${prefix}fdump(FILE *f, ${type} A, char *name) {
	int i, j;
	if (f == NULL) f = stdout;
	if (name) fprintf(f, "%s:\\n",  name);
	for (i = 0; i < matSIZE; i++) {
		fprintf(f, " | ");
		for (j = 0; j < matSIZE; j++) fprintf(f, "%6.2f ", A.a[i][j]);			
		fprintf(f, "\\n");
	}
}

EOB
;

#
# mat add:
print C "${type}\n${prefix}add(${type} A, ${type} B) {\n";
print C "\tregister $type C;\n";
print C "\tC = __${prefix}null;\n\n";
for ($i = 0; $i < ${size}; $i++) {
   	for ($j = 0; $j < ${size}; $j++) {
		print C "\tC.a[$i][$j] =  A.a[$i][$j] + B.a[$i][$j};\n";
	}
	print C "\n";
}
print C"\treturn C;\n}\n\n";

#
# mat sub:
print C "${type}\n${prefix}sub(${type} A, ${type} B) {\n";
print C "\tregister $type C;\n";
print C "\tC = __${prefix}null;\n\n";
for ($i = 0; $i < ${size}; $i++) {
   	for ($j = 0; $j < ${size}; $j++) {
		print C "\tC.a[$i][$j] =  A.a[$i][$j] - B.a[$i][$j};\n";
	}
	print C "\n";
}
print C"\treturn C;\n}\n\n";

#
# mat mul:
print C "${type}\n${prefix}mul(${type} A, ${type} B) {\n";
print C "\tregister ${type} C;\n";
#print C "\tC = __${prefix}null;\n";
for ($i = 0; $i < ${size}; $i++) {
   	for ($j = 0; $j < ${size}; $j++) {
        print C "\tC.a[$i][$j] = ";
        for ($k  = 0; $k < ${size}; $k++) {
			if ($k > 0) {
				print C " + ";
			}
   			print C "A.a[$i][$k] * B.a[$k][$j]";
        } 
		print C ";\n";
	}
}
print C "\treturn C;\n}\n\n";

#
# Mat transpose:
print C "${type}\n${prefix}transpose(${type} A) {\n";
print C "\tregister ${type} B;\n";
for ($i = 0; $i < $size; $i++) {
   	for ($j = 0; $j < $size; $j++) {
		if ($j == 0) {
			print C "\t";
		} else {
			print C " ";
		}
    	print C "B.a[$i][$j] = A.a[$j][$i];";
	}
	print C "\n";
}
print C "\treturn B;\n}\n";

#
# mat det:
#printf C "double\n${prefix}det(${type} A) {\n\treturn\n";
#$s1 = 1;
#for ($i = 0; $i < ${size}; $i++) {
#	for ($j = 0; $j < ${size}; $j++) {
#		print C A.a[$i][$j] *
#	}
#}
#
#double ${prefix}det(${type} A) {
#    double c, r = 1;
#	int i, j, k;
#
#   for (i = 0; i < ${size}; i++) {
#        for (k = i + 1; k < ${size}; k++) {
#            c = A[k][i] / A[i][i];
#            for (j = i; j < ${size}; j++) {
#                A[k][j] = A[k][j] - c * A[i][j];
#			} 
#        }
#    }
#    for (i = 0; i < ${size}; i++) {
#        r *= A[i][i];
#	}
#
#    return r;
#}




close(C);
