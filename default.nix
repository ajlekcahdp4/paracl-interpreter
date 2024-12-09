{
  pkgs,
  stdenv,
  lib,
  ...
}:
let
  fs = lib.fileset;
in
stdenv.mkDerivation {
  pname = "llvm-bleach";
  version = "0.0.0";
  src = fs.toSource {
    root = ./.;
    fileset = fs.unions [
      ./CMakeLists.txt
      ./src
      ./include
      ./test
      ./scripts
    ];
  };
  nativeBuildInputs = with pkgs; [
    cmake
    bison
    flex
    fmt
  ];
  buildInputs = with pkgs; [ boost ];
}
