# Copyright 1999-2004 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /cvsroot/libcddb/libcddb/misc/libcddb.ebuild,v 1.7 2004/10/08 21:18:07 airborne Exp $

IUSE="doc"

DESCRIPTION="A library for accessing a CDDB server"
HOMEPAGE="http://libcddb.sourceforge.net/"
SRC_URI="mirror://sourceforge/${PN}/${P}.tar.gz"
RESTRICT="nomirror"
LICENSE="LGPL-2"

DEPEND=">=dev-libs/libcdio-0.67
	    doc? ( app-doc/doxygen )"

SLOT="0"
KEYWORDS="~x86"


src_compile() {
	econf || die
	emake || die

	# Create API docs if needed and possible
	if use doc; then
		cd doc
		doxygen doxygen.conf
	fi
}

src_install() {
	make DESTDIR=${D} install

	dodoc AUTHORS ChangeLog COPYING INSTALL NEWS README THANKS TODO
	# Create API docs if needed and possible
	use doc && dohtml doc/html/*
}
