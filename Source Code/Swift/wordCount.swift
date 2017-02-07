type filee ;
type count;
app (count t) countwords (filee f) {
wc "-w" @filename(f) stdout=@filename(t);
}
string inputName = "wiki10gb.txt";
filee inputs[] <mapper;files=inputName>;
foreach f in inputs {
count c<regularexpressionmapper;source=@f, match="(.*)txt",transform="\\1count">;
c = countwords(f);
}