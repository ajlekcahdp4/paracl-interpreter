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
      flake.overlays.default = _final: prev: {
        llvmPackages_19.libllvm = prev.llvmPackages_19.libllvm.overrideAttrs {
          patches = prev.patches ++ [ ./overlays/llvm-install-target-headers.patch ];
        };
      };
      perSystem =
        { pkgs, ... }:
        let
          llvmPkgs = pkgs.llvmPackages_19;
        in
        rec {
          imports = [ ./nix/treefmt.nix ];
          packages = rec {
            paracl = pkgs.callPackage ./. { inherit (llvmPkgs) stdenv; };
          };
          devShells.default = pkgs.mkShell {
            nativeBuildInputs =
              packages.paracl.nativeBuildInputs
              ++ (with pkgs; [
                clang-tools
                filecheck
                act
                gdb
                valgrind
                just
              ]);
            buildInputs = packages.paracl.buildInputs;
          };
        };
    };
}
