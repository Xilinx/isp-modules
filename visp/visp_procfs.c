// SPDX-License-Identifier: MIT
/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 * The GPL License (GPL)
 *
 * Copyright (c) 2025 VeriSilicon Holdings Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 *****************************************************************************
 *
 * Note: This software is released under dual MIT and GPL licenses. A
 * recipient may use this file under the terms of either the MIT license or
 * GPL License. If you wish to use only one license not the other, you can
 * indicate your decision by deleting one of the above license notices in your
 * version of this file.
 *
 *****************************************************************************/

#include <linux/ctype.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include "visp_driver.h"
#include "visp_procfs.h"

struct visp_procfs {
	struct proc_dir_entry *pde;
	struct visp_dev *isp_dev;
	struct mutex lock;
};

static char *type2str(int type)
{
	switch (type) {
	case VISP_PATH_OUT_TYPE_MEMORY:
		return "memory";

	case VISP_PATH_OUT_TYPE_STREAM:
		return "stream";

	default:
		return "invalid";
	}
}

static int visp_procfs_info_show(struct seq_file *sfile, void *offset)
{
	struct visp_procfs *isp_proc;
	struct visp_dev *isp_dev;
	struct media_pad *pad = NULL;
	int pad_idx = 0;
	int port = -1;

	isp_proc = (struct visp_procfs *)sfile->private;

	mutex_lock(&isp_proc->lock);
	isp_dev = isp_proc->isp_dev;

	seq_printf(sfile, "/******isp configuration******/\n");
	for (pad_idx = 0; pad_idx < isp_dev->num_pads; pad_idx++) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
		pad = media_pad_remote_pad_first(&isp_dev->pads[pad_idx]);
#else
		pad = media_entity_remote_pad(&isp_dev->pads[pad_idx]);
#endif

		if (pad && (port != (pad_idx / VISP_PORT_PAD_NR))) {
			port = pad_idx / VISP_PORT_PAD_NR;
			seq_printf(sfile, "isp%d port%d:\n", isp_dev->id, port);
			seq_printf(sfile, "output_type: %s %s %s %s\n",
				   type2str(isp_dev->output_type[port][0]),
				   type2str(isp_dev->output_type[port][1]),
				   type2str(isp_dev->output_type[port][2]),
				   type2str(isp_dev->output_type[port][3]));
			seq_printf(sfile, "sensor      : %s\n",
				   isp_dev->isp_ports[port].sensor_info.name);
			seq_printf(sfile, "mode        : %u\n",
				   isp_dev->isp_ports[port].sensor_info.mode);
			seq_printf(
			    sfile, "calib       : %s\n",
			    isp_dev->isp_ports[port].sensor_info.calib);
			seq_printf(
			    sfile, "manu_json   : %s\n",
			    isp_dev->isp_ports[port].sensor_info.manu_json);
			seq_printf(
			    sfile, "auto_json   : %s\n",
			    isp_dev->isp_ports[port].sensor_info.auto_json);
			seq_printf(
			    sfile, "one_json    : %s\n",
			    isp_dev->isp_ports[port].sensor_info.one_json);
			int iba_index = 0;

			if ((isp_dev->id % 2) == 0)
				iba_index = port;
			else if ((isp_dev->id % 2) == 1)
				iba_index = (isp_dev->num_streams == 1) ? 4 : 4 - port;

			seq_printf(sfile, "vc_id       : %u\n",
				   isp_dev->iba[iba_index].vcid);
			seq_printf(
			    sfile, "sensor_id   : %u\n",
			    isp_dev->isp_ports[port].sensor_info.sensor_id);
			seq_printf(sfile, "i2c_id      : %u\n",
				   isp_dev->isp_ports[port].sensor_info.i2c_id);
			seq_printf(sfile, "buffer_type : %s\n",
				   isp_dev->isp_ports[port].bufmode);
			seq_printf(sfile, "hw_mcm      : %s\n",
					isp_dev->isp_ports[port].hw_mcm ? "1 / Enable" : "0 / Disable" );
			seq_printf(sfile, "fusa_json   : %s\n",
				   isp_dev->isp_ports[port].fusa_json);

			/* Display LLP status, capability, and which path it's enabled for */
			{
				int llp_path = -1;
				int capable_path = -1;
				int i;
				const char *path_names[] = {"MP", "SP1", "SP2", "RAW"};

				/* Find which path is enabled */
				for (i = 0; i < 4; i++) {
					if (ISP_DEV_EXTENDED(isp_dev)->llp[i]) {
						llp_path = i;
						break;
					}
				}

				/* Find which path is capable (from device tree) */
				for (i = 0; i < 4; i++) {
					if (ISP_DEV_EXTENDED(isp_dev)->llp_capable[i]) {
						capable_path = i;
						break;
					}
				}

				/* Display status */
				if (llp_path >= 0) {
					seq_printf(sfile, "llp         : Enabled (Path: %s/%d, Capable: %s)\n",
						path_names[llp_path], llp_path,
						capable_path >= 0 ? path_names[capable_path] : "None");
				} else {
					seq_printf(sfile, "llp         : Disabled (Capable: %s)\n",
						capable_path >= 0 ? path_names[capable_path] : "None");
				}
			}

			seq_printf(sfile,
				   "#################################\n\n\n");
		}
	}

	mutex_unlock(&isp_proc->lock);

	return 0;
}

static int visp_procfs_open(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
	return single_open(file, visp_procfs_info_show, PDE_DATA(inode));
#else
	return single_open(file, visp_procfs_info_show, pde_data(inode));
#endif
}

static int32_t visp_proc_process(struct seq_file *sfile,
				 struct visp_procfs *isp_proc, char *str_buf)
{
	struct visp_dev *isp_dev;
	char *token = NULL, *cur = str_buf;
	char *val = NULL, *kv_cur = NULL, *end = NULL;
	char *const delim = " ";
	char *const kv_delim = "=";
	char *const com_delim = ",";
	int port = 0;
	int path = 0;

	isp_dev = isp_proc->isp_dev;

	mutex_lock(&isp_proc->lock);

	while ((token = strsep(&cur, delim))) {
		if (isdigit(*token)) {
			port = *token - '0';
			continue;
		}

		if ((port > VISP_PORT_NR) || (port < 0))
			continue;

		kv_cur = token;
		val = strsep(&kv_cur, kv_delim);
		if (val) {
			if (strcmp(val, "sensor") == 0) {
				val = strsep(&kv_cur, kv_delim);
				if (val) {
					memset(isp_dev->isp_ports[port]
						   .sensor_info.name,
					       0,
					       sizeof(isp_dev->isp_ports[port]
							  .sensor_info.name));
					strscpy(isp_dev->isp_ports[port]
						    .sensor_info.name,
						val,
						sizeof(isp_dev->isp_ports[port]
						       .sensor_info.name));
				}
			} else if (strcmp(val, "mode") == 0) {
				val = strsep(&kv_cur, kv_delim);
				if (val && isdigit(*val)) {
					isp_dev->isp_ports[port]
					    .sensor_info.mode =
					    (uint32_t)simple_strtoul(val, &end,
								     0);
				}
			} else if (strcmp(val, "calib") == 0) {
				val = strsep(&kv_cur, kv_delim);
				if (val) {
					memset(
					    isp_dev->isp_ports[port]
						.sensor_info.calib,
					    0,
					    sizeof(isp_dev->isp_ports[port]
						       .sensor_info.calib));
					strscpy(isp_dev->isp_ports[port]
						    .sensor_info.calib,
						val,
						sizeof(isp_dev->isp_ports[port]
						       .sensor_info.calib));
				}
			} else if (strcmp(val, "manu_json") == 0) {
				val = strsep(&kv_cur, kv_delim);
				if (val) {
					memset(
					    isp_dev->isp_ports[port]
						.sensor_info.manu_json,
					    0,
					    sizeof(isp_dev->isp_ports[port]
						       .sensor_info.manu_json));
					strscpy(isp_dev->isp_ports[port]
						    .sensor_info.manu_json,
						val,
						sizeof(isp_dev->isp_ports[port]
						       .sensor_info.manu_json));
				}
			} else if (strcmp(val, "auto_json") == 0) {
				val = strsep(&kv_cur, kv_delim);
				if (val) {
					memset(
					    isp_dev->isp_ports[port]
						.sensor_info.auto_json,
					    0,
					    sizeof(isp_dev->isp_ports[port]
						       .sensor_info.auto_json));
					strscpy(isp_dev->isp_ports[port]
						    .sensor_info.auto_json,
						val,
						sizeof(isp_dev->isp_ports[port]
						       .sensor_info.auto_json));
				}
			} else if (strcmp(val, "one_json") == 0) {
				val = strsep(&kv_cur, kv_delim);
				if (val) {
					memset(
					    isp_dev->isp_ports[port]
						.sensor_info.one_json,
					    0,
					    sizeof(isp_dev->isp_ports[port]
						       .sensor_info.one_json));
					strscpy(isp_dev->isp_ports[port]
						    .sensor_info.one_json,
						val,
						sizeof(isp_dev->isp_ports[port]
						       .sensor_info.one_json));
				}
			} else if (strcmp(val, "output_type") == 0) {
				val = strsep(&kv_cur, kv_delim);
				if (val) {
					while (
					    (token = strsep(&val, com_delim))) {
						if (path <
							VISP_PORT_PAD_NR - 1 &&
						    isdigit(*token)) {
							isp_dev->output_type
							    [port][path] =
							    *token - '0';
							path++;
						}
					}
				}
			} else if (strcmp(val, "vc_id") == 0) {
				val = strsep(&kv_cur, kv_delim);
				if (val && isdigit(*val)) {
					int iba_index = 0;

					if ((isp_dev->id % 2) == 0)
						iba_index = port;
					else if ((isp_dev->id % 2) == 1)
						iba_index =
						(isp_dev->num_streams == 1) ? 4 : 4 - port;

					isp_dev->iba[iba_index].vcid =
					    (uint32_t)simple_strtoul(val, &end,
								     0);
				}
			} else if (strcmp(val, "sensor_id") == 0) {
				val = strsep(&kv_cur, kv_delim);
				if (val && isdigit(*val)) {
					isp_dev->isp_ports[port]
					    .sensor_info.sensor_id =
					    (uint32_t)simple_strtoul(val, &end,
								     0);
				}
			} else if (strcmp(val, "i2c_id") == 0) {
				val = strsep(&kv_cur, kv_delim);
				if (val && isdigit(*val)) {
					isp_dev->isp_ports[port]
					    .sensor_info.i2c_id =
					    (uint32_t)simple_strtoul(val, &end,
								     0);
				}
			} else if (strcmp(val, "bufmode") == 0) {
				val = strsep(&kv_cur, kv_delim);
				if (val) {
					memset(isp_dev->isp_ports[port].bufmode,
					       0,
					       sizeof(isp_dev->isp_ports[port]
							  .bufmode));
					strscpy(
					    isp_dev->isp_ports[port].bufmode,
					    val,
					    sizeof(isp_dev->isp_ports[port].bufmode));
				}
			} else if (strcmp(val, "hw_mcm") == 0) {
				val = strsep(&kv_cur, kv_delim);
				if (val && isdigit(*val)) {
					if((uint32_t)simple_strtoul(val, &end, 0) == 1 )
					{
						isp_dev->isp_ports[port].hw_mcm=(bool)true;
					}
					else
					{
						isp_dev->isp_ports[port].hw_mcm=(bool)false;
					}
				}
			} else if (strcmp(val, "fusa_json") == 0) {
				val = strsep(&kv_cur, kv_delim);
				if (val) {
					memset(isp_dev->isp_ports[port].fusa_json, 0,
					       sizeof(isp_dev->isp_ports[port].fusa_json));

					strscpy(isp_dev->isp_ports[port].fusa_json,
						val, sizeof(isp_dev->isp_ports[port].fusa_json));
				}
			} else if (strcmp(val, "llp") == 0) {
			    val = strsep(&kv_cur, kv_delim);
			    if (val) {
				    int i;
				    int path = -1;
				    const char *path_names[] = {"MP", "SP1", "SP2", "RAW"};

				    /* Check for "off" or "disable" to disable all capable paths */
				    if (strcasecmp(val, "off") == 0 || strcasecmp(val, "disable") == 0) {
					    for (i = 0; i < 4; i++) {
						    if (ISP_DEV_EXTENDED(isp_dev)->llp_capable[i]) {
							    ISP_DEV_EXTENDED(isp_dev)->llp[i] = 0;
						    }
					    }
					    dev_info(isp_dev->dev, "LLP disabled for all capable paths\n");
				    }
				    /* Check for path names (case-insensitive) */
				    else if (strcasecmp(val, "mp") == 0 || strcasecmp(val, "main") == 0) {
					    path = 0;
				    }
				    else if (strcasecmp(val, "sp") == 0 || strcasecmp(val, "sp1") == 0 ||
					     strcasecmp(val, "self") == 0 || strcasecmp(val, "self1") == 0) {
					    path = 1;
				    }
				    else if (strcasecmp(val, "sp2") == 0 || strcasecmp(val, "self2") == 0) {
					    path = 2;
				    }
				    else if (strcasecmp(val, "raw") == 0) {
					    path = 3;
				    }
				    /* Check for numeric value (0-3) to enable specific path */
				    else if (isdigit(*val)) {
					    path = simple_strtoul(val, &end, 0);
					    if (path >= 4) {
						    dev_warn(isp_dev->dev,
							    "Invalid LLP path %d (must be 0-3)\n", path);
						    path = -1;
					    }
				    }
				    else {
					    dev_warn(isp_dev->dev,
						    "Invalid LLP value '%s' (use: mp/sp1/sp2/raw, 0-3, or off)\n",
						    val);
				    }

				    /* Enable the selected path if valid and capable */
				    if (path >= 0 && path < 4) {
					    /* Check if this path is LLP-capable from device tree */
					    if (!ISP_DEV_EXTENDED(isp_dev)->llp_capable[path]) {
						    dev_warn(isp_dev->dev,
							    "LLP path %d (%s) is not capable (not configured in device tree)\n",
							    path, path_names[path]);
					    } else {
						    /* First disable all capable paths */
						    for (i = 0; i < 4; i++) {
							    if (ISP_DEV_EXTENDED(isp_dev)->llp_capable[i]) {
								    ISP_DEV_EXTENDED(isp_dev)->llp[i] = 0;
							    }
						    }
						    /* Then enable the specified path */
						    ISP_DEV_EXTENDED(isp_dev)->llp[path] = 1;
						    dev_info(isp_dev->dev, "LLP enabled for path %d (%s)\n",
							    path, path_names[path]);
					    }
				    }
			    }
			}
}
	}

	mutex_unlock(&isp_proc->lock);

	return 0;
}

static ssize_t visp_procfs_write(struct file *file, const char __user *buffer,
				 size_t count, loff_t *ppos)
{
	struct visp_procfs *isp_proc;
	struct seq_file *sfile;
	char *str_buf;

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
	isp_proc = (struct visp_procfs *)PDE_DATA(file_inode(file));
#else
	isp_proc = (struct visp_procfs *)pde_data(file_inode(file));
#endif
	sfile = file->private_data;

	str_buf = (char *)kzalloc(count, GFP_KERNEL);
	if (!str_buf)
		return -ENOMEM;

	if (copy_from_user(str_buf, buffer, count))
		return -EFAULT;

	*(str_buf + count - 1) = '\0';

	visp_proc_process(sfile, isp_proc, str_buf);

	kfree(str_buf);

	return count;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 6, 0)
static const struct file_operations visp_procfs_ops = {
	.open = visp_procfs_open,
	.release = seq_release,
	.read = seq_read,
	.write = visp_procfs_write,
	.llseek = seq_lseek,
};
#else
static const struct proc_ops visp_procfs_ops = {
	.proc_open = visp_procfs_open,
	.proc_release = seq_release,
	.proc_read = seq_read,
	.proc_write = visp_procfs_write,
	.proc_lseek = seq_lseek,
};
#endif

struct finddir_callback {
	struct dir_context ctx;
	const char *name;
	int32_t files_cnt;
	bool found;
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0)
static int readdir_callback(struct dir_context *ctx, const char *name,
			    int namelen, loff_t offset, u64 ino,
			    unsigned int d_type)
{
	struct finddir_callback *fc =
	    container_of(ctx, struct finddir_callback, ctx);
	if (fc->found)
		return 0;

	if (strcmp(name, fc->name) == 0)
		fc->found = true;
	fc->files_cnt++;
	return 0;
}

#else
static bool readdir_callback(struct dir_context *ctx, const char *name,
			     int namelen, loff_t offset, u64 ino,
			     unsigned int d_type)
{
	struct finddir_callback *fc =
	    container_of(ctx, struct finddir_callback, ctx);
	if (fc->found)
		return true;
	if (strcmp(name, fc->name) == 0)
		fc->found = true;
	fc->files_cnt++;
	return true;
}
#endif

static int find_proc_dir_by_name(const char *root, const char *name,
				 bool *found, int32_t *files_cnt)
{
	struct file *pfile;
	int ret = 0;
	struct finddir_callback fc = {
	    .ctx.actor = readdir_callback,
	    .name = name,
	    .found = false,
	    .files_cnt = -2,
	};

	pfile = filp_open(root, O_RDONLY | O_DIRECTORY, 0);
	if (pfile->f_op->iterate_shared)
		ret = pfile->f_op->iterate_shared(pfile, &fc.ctx);
/*	else {
 *		//ret = pfile->f_op->iterate(pfile, &fc.ctx);
 */
	if (ret == 0)
		*found = fc.found;

	if (files_cnt != NULL)
		*files_cnt = fc.files_cnt;

	filp_close(pfile, NULL);
	return ret;
}

int visp_procfs_register(struct visp_dev *isp_dev, unsigned long *pde)
{
	struct visp_procfs *isp_proc;
	char isp_proc_name[32];
	int ret = 0;
	bool found = false;

	if (!isp_dev)
		return -1;
	sprintf(isp_proc_name, "vsi/isp_subdev%d", isp_dev->id);

	isp_proc =
	    devm_kzalloc(isp_dev->dev, sizeof(struct visp_procfs), GFP_KERNEL);

	if (!isp_proc)
		return -ENOMEM;

	ret = find_proc_dir_by_name("/proc", "vsi", &found, NULL);
	if (ret == 0) {
		if (!found)
			proc_mkdir("vsi", NULL);
	} else {
		return -EFAULT;
	}

	isp_proc->isp_dev = isp_dev;
	isp_proc->pde = proc_create_data(isp_proc_name, 0664, NULL,
					 &visp_procfs_ops, isp_proc);
	if (!isp_proc->pde)
		return -EFAULT;
	*pde = (unsigned long)&isp_proc->pde;

	mutex_init(&(isp_proc->lock));
	return 0;
}

void visp_procfs_unregister(unsigned long pde)
{
	int ret = 0;
	bool found = false;
	int32_t files_cnt;
	struct visp_procfs *isp_proc = (struct visp_procfs *)pde;

	ret = find_proc_dir_by_name("/proc", "vsi", &found, NULL);
	if (ret == 0) {
		if (found) {
			proc_remove(isp_proc->pde);
			ret = find_proc_dir_by_name("/proc/vsi", "", &found,
						    &files_cnt);
			if (files_cnt == 0)
				remove_proc_subtree("vsi", NULL);
		}
	}
}
