make
fld=$1
n=$2
p=$3

for ((i=1 ; i<=5; i++))
do
	rm -rf ./$fld/files/client$i/Downloaded/
done
mkdir -p output
for ((i=1 ; i<=$n ; i++))
do
	timeout 60 ./client-phase$p ./$fld/client$i-config.txt ./$fld/files/client$i/ > ./output/op-c$i-p$p.txt &
done