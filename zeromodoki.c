#include <sys/cdefs.h>
__FBSDID("$FreeBSD: release/9.1.0/sys/dev/null/null.c 230320 2012-01-18 21:54:34Z gnn $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/priv.h>
#include <sys/disk.h>
#include <sys/bus.h>
#include <sys/filio.h>

#include <machine/bus.h>
#include <machine/vmparam.h>

/* For use with destroy_dev(9). */
static struct cdev *zeromodoki_dev;

static d_write_t zeromodoki_write;
static d_ioctl_t zeromodoki_ioctl;
static d_read_t zeromodoki_read;

static struct cdevsw zeromodoki_cdevsw = {
	.d_version =	D_VERSION,
	.d_read =	zeromodoki_read,
	.d_write =	zeromodoki_write,
	.d_ioctl =	zeromodoki_ioctl,
	.d_name =	"zeromodoki",
	.d_flags =	D_MMAP_ANON,
};

/* ARGSUSED */
static int
zeromodoki_write(struct cdev *dev __unused, struct uio *uio, int flags __unused)
{
	uio->uio_resid = 0;

	return (0);
}

/* ARGSUSED */
static int
zeromodoki_ioctl(struct cdev *dev __unused, u_long cmd, caddr_t data __unused,
	   int flags __unused, struct thread *td)
{
	int error;
	error = 0;

	switch (cmd) {
	case FIONBIO:
		break;
	case FIOASYNC:
		if (*(int *)data != 0)
			error = EINVAL;
		break;
	default:
		error = ENOIOCTL;
	}
	return (error);
}


/* ARGSUSED */
static int
zeromodoki_read(struct cdev *dev __unused, struct uio *uio, int flags __unused)
{
	void *zbuf;
	ssize_t len;
	int error = 0;

	KASSERT(uio->uio_rw == UIO_READ,
	    ("Can't be in %s for write", __func__));
	zbuf = __DECONST(void *, zero_region);
	while (uio->uio_resid > 0 && error == 0) {
		len = uio->uio_resid;
		if (len > ZERO_REGION_SIZE)
			len = ZERO_REGION_SIZE;
		error = uiomove(zbuf, len, uio);
	}

	return (error);
}

/* ARGSUSED */
static int
null_modevent(module_t mod __unused, int type, void *data __unused)
{
	switch(type) {
	case MOD_LOAD:
		if (bootverbose)
			printf("zeromodoki: <zeromodoki device>\n");
		zeromodoki_dev = make_dev_credf(MAKEDEV_ETERNAL_KLD, &zeromodoki_cdevsw, 0,
		    NULL, UID_ROOT, GID_WHEEL, 0666, "zeromodoki");
		break;

	case MOD_UNLOAD:
		destroy_dev(zeromodoki_dev);
		break;

	case MOD_SHUTDOWN:
		break;

	default:
		return (EOPNOTSUPP);
	}

	return (0);
}

DEV_MODULE(zeromodoki, null_modevent, NULL);
MODULE_VERSION(zeromodoki, 1);
