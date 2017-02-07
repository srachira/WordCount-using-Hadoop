type filee;
type count;
app (count t) countwords (filee f) {
wc "-w" @filename(f) stdout=@filename(t);}
messagefile inputfile <"regularexpressionmapper.words.txt">;
countfile c <regularexpressionmapper; source=@inputfile,match="(.*)txt",transform="\\1count">;
c =countwords(inputfile);