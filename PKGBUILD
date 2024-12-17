# Maintainer: Mariusz Kuchta <mariusz@kuchta.dev>
pkgname=marbs-themetime
pkgrel=1
pkgver=0.1.0
pkgdesc="automatically set theme based on sunset/rise"
arch=('x86_64')
url="https://github.com/Kuchteq/marbs-themetime"
license=('GPL')
makedepends=('git' 'make')
provides=("${pkgname%-git}")
conflicts=("${pkgname%-git}")

build() {
	cd ../
	make
}

package() {
	cd ../
	DESTDIR="$pkgdir/" PREFIX=/usr make install
}
