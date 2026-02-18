{
	description = "es-shell development environment";

	inputs = {
		nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
	};

	outputs = { self, nixpkgs }:
		let
			systems = [
				"aarch64-darwin"
				"x86_64-darwin"
				"aarch64-linux"
				"x86_64-linux"
			];

			forAllSystems = f:
				nixpkgs.lib.genAttrs systems (system:
					let
						pkgs = import nixpkgs { inherit system; };
						es = pkgs.stdenv.mkDerivation {
							pname = "es";
							version = "0.10.0";
							src = self;
							nativeBuildInputs = with pkgs; [
								autoreconfHook
								bison
								flex
								gnumake
								libtool
								pkg-config
							];
							buildInputs = with pkgs; [
								ncurses
								readline
							];
							preConfigure = ''
								export AWK=${pkgs.gawk}/bin/awk
								export INSTALL=${pkgs.coreutils}/bin/install
							'';
							configureFlags = [
								"--with-readline"
							];
							enableParallelBuilding = true;
							doCheck = false;
						};
					in
					f { inherit pkgs es; }
				);
		in
		{
			packages = forAllSystems ({ es, ... }: {
				default = es;
				es = es;
			});

			checks = forAllSystems ({ es, ... }: {
				build = es;
			});

			devShells = forAllSystems ({ pkgs, ... }: {
				default = pkgs.mkShell {
					packages = with pkgs; [
						autoconf
						automake
						bison
						flex
						gcc
						gawk
						gnumake
						libtool
						ncurses
						pkg-config
						procps
						readline
					];
					shellHook = ''
						export AWK=awk
						export INSTALL=${pkgs.coreutils}/bin/install
						export PATH=${pkgs.procps}/bin:$PATH
					'';
				};
			});
		};
}
