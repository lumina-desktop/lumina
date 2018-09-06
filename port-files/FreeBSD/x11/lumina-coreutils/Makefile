# $FreeBSD$

PORTNAME=	lumina-coreutils
PORTVERSION=	%%CHGVERSION%%
PORTREVISION=	%%CHGREVISION%%
CATEGORIES=	x11

MAINTAINER=	jt@ixsystems.com
COMMENT=	Lumina Desktop Environment

LICENSE=	BSD3CLAUSE

RUN_DEPENDS=	xrandr:x11/xrandr

CONFLICTS_INSTALL=	lumina-1.2.*

OPTIONS_DEFINE=	I18N
OPTIONS_SUB=		yes
OPTIONS_DEFAULT=	I18N
I18N_DESC= Install localization files

.include <bsd.port.options.mk>

MAKE_JOBS_UNSAFE=yes

USES=		qmake qt:5

USE_LDCONFIG=	yes
USE_QT=			core gui widgets network svg imageformats \
		buildtools_build x11extras concurrent multimedia

USE_GITHUB=	yes
GH_ACCOUNT=	trueos
GH_PROJECT=	lumina
GH_TAGNAME=	%%GHTAG%%
WRKSRC_SUBDIR=	src-qt5/core-utils

.if${PORT_OPTIONS:MI18N}
USE_QT+= linguist_build
QMAKE_ARGS+=CONFIG+=WITH_I18N
.endif

.include <bsd.port.mk>
