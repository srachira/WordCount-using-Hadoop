type filee;
type count;
app (count t) countwords(filee f) {
wc "-w" @filename(f) stdout=@filename(t);
}
string inputNames = "mapper1.txt";
string outputNames = "mapper1.count ";
filee inputfiles[] <mapper;files=inputNames>;
count outputfiles[] <mapper;files=outputNames>;
outputfiles[0] = countwords(inputfiles[0]);