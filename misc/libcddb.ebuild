# Copyright 1999-2003 Gentoo Technologies, Inc.
# Distributed under the terms of the GNU General Public License v2
# $Header: /cvsroot/libcddb/libcddb/misc/libcddb.ebuild,v 1.2 2003/04/17 22:14:24 airborne Exp $

IUSE="doc"

DESCRIPTION="A library for accessing a CDDB server"
HOMEPAGE="http://libcddb.sourceforge.net/"
SRC_URI="mirror://sourceforge/${PN}/${P}.tar.gz"
LICENSE="LGPL"

SLOT="0"
KEYWORDS="x86"


src_compile() {
	econf || die
	emake || die

	# Create API docs if needed and possible
	if [ `use doc` -a `has_version 'app-doc/doxygen'` ]; then
		cd doc
		doxygen doxygen.conf
	fi
}

src_install() {
	make DESTDIR=${D} install

	dodoc AUTHORS Changelog COPYING INSTALL NEWS README THANKS TODO
	# Create API docs if needed and possible
	if [ `use doc` -a `has_version 'app-doc/doxygen'` ]; then
		dohtml doc/html/*
	fi
}
