.SUFFIXES:	.bdf .snf 

FONTS= 6x12.snf \
7x13.snf \
7x14.snf \
8x13.snf  \
8x16.snf  \
9x15.snf  \
courB10.snf  \
courB12.snf  \
courB14.snf  \
courB18.snf  \
courB24.snf  \
courBO10.snf  \
courBO12.snf  \
courBO14.snf  \
courBO18.snf  \
courBO24.snf  \
courO10.snf  \
courO12.snf  \
courO14.snf  \
courO18.snf  \
courO24.snf  \
courR10.snf  \
courR12.snf  \
courR14.snf  \
courR18.snf  \
courR24.snf  \
helvB10.snf  \
helvB12.snf  \
helvB14.snf  \
helvB18.snf  \
helvB24.snf  \
helvBO10.snf  \
helvBO12.snf  \
helvBO14.snf  \
helvBO18.snf  \
helvBO24.snf  \
helvO10.snf  \
helvO12.snf  \
helvO14.snf  \
helvO18.snf  \
helvO24.snf  \
helvR10.snf  \
helvR12.snf  \
helvR14.snf  \
helvR18.snf  \
helvR24.snf  \
symb10.snf  \
symb12.snf  \
symb14.snf  \
symb18.snf  \
symb24.snf  

xfonts.dir : $(FONTS)
	./mkxfdir *.snf > xfonts.dir

.bdf.snf :
	./bdftosnf $*.bdf > $*.snf
	

