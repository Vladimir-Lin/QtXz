function CompressXZ()
{
  xz = new QtXZ() ;
  xz . ToXz ( "Testing.txt" , "Testing.xz" , 9 ) ;
  delete xz ;
}

function DecompressLZO()
{
  xz = new QtXz() ;
  xz . ToFile ( "Testing.xz" , "Testing.txt" ) ;
  delete xz ;
}
