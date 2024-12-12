{
  description = "paracl";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
    treefmt-nix = {
      url = "github:numtide/treefmt-nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    {
      flake-parts,
      treefmt-nix,
      ...
    }@inputs:
    flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [ treefmt-nix.flakeModule ];

      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];
      perSystem =
        { pkgs, ... }:
        rec {
          imports = [ ./nix/treefmt.nix ];
          packages = rec {
            paracl = pkgs.callPackage ./. { stdenv = pkgs.gcc14Stdenv; };
          };
          devShells.default = (pkgs.mkShell.override { stdenv = pkgs.gcc14Stdenv; }) {
            nativeBuildInputs =
              packages.paracl.nativeBuildInputs
              ++ (with pkgs; [
                clang-tools
                filecheck
                act
                gdb
                lldb
                libffi
                valgrind
                just
              ]);
            buildInputs = packages.paracl.buildInputs;
          };
        };
    };
}
