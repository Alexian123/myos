#!/usr/bin/bash

# exit on error
set -e

# print usage
if [ $# -le 1 ]; then
    echo "Usage: $0 <isrs_gen.c> <isrs_gen.inc>"
    exit 1
fi

# vars
ISRS_GEN_C=$1
ISRS_GEN_ASM=$2
ISRS_WITH_ERROR_CODE="8 10 11 12 13 14 17 21 29 30" # check intel manual

# generate C file
echo "// ! THIS FILE IS GENERATED AUTOMATICALLY !" > $ISRS_GEN_C
echo "#include \"idt.h\"" >> $ISRS_GEN_C
echo "#include \"gdt.h\"" >> $ISRS_GEN_C
echo "" >> $ISRS_GEN_C
for i in $(seq 0 255); do
    echo "extern void __attribute__((cdecl)) i686_isr${i}();" >> $ISRS_GEN_C
done
echo "" >> $ISRS_GEN_C
echo "void i686_ISR_initGates() {" >> $ISRS_GEN_C
for i in $(seq 0 255); do
    echo "    i686_IDT_setGate(${i}, i686_isr${i}, i686_GDT_CODE_SEGMENT_OFFSET, IDT_FLAG_RING0 | IDT_FLAG_GATE_32BIT_INT);" >> $ISRS_GEN_C
done
echo "}" >> $ISRS_GEN_C

# generate ASM file
echo "; ! THIS FILE IS GENERATED AUTOMATICALLY !" > $ISRS_GEN_ASM
for i in $(seq 0 255); do
    if echo "$ISRS_WITH_ERROR_CODE" | grep -q "\b${i}\b"; then
        echo "ISR_WITH_ERROR_CODE ${i}" >> $ISRS_GEN_ASM
    else
        echo "ISR_NO_ERROR_CODE ${i}" >> $ISRS_GEN_ASM
    fi
done

exit 0