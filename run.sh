conda activate core-v-mini-mcu
export RISCV=/home/linus/tools/riscv 
make app PROJECT=app
make run-helloworld