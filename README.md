# 基于 QT 的嵌入式 Linux 驱动综合测试及应用平台

## 1、拉取仓库代码

拉取仓库代码，文件系统大概 550MB，因此需要拉取大文件，按照下面步骤操作即可

```bash
# 克隆仓库
git clone git@github.com:lc-guo/QT-I.MX6ULL.git
# 进入仓库目录
cd QT-I.MX6ULL
# 安装 LFS
git lfs install
# 拉取大文件
git lfs pull
```

## 2、制作 SD 卡启动盘

制作 SD 卡启动盘，SD 卡应该有 3 个分区，并且分区中存放的内容一致

1. 分区 0 对应 sdx，用于存放 u-boot.imx
2. 分区 1 对应 sdx1，格式为 vfat，用于存放 Linux 镜像和 .dtb 设备树文件
3. 分区 2 对应 sdx2，格式为 ext4，用于存放根文件系统（rootfs）

首先在 windwos 下格式化整个 SD 卡，进入 CMD ，输入以下命令对 SD 卡格式化

```bash
# 进入磁盘管理
diskport
# 列举所有磁盘
list disk
# 选择 SD 卡磁盘
select disk n
# 清除磁盘内容
clean
# 创建原始分区
create partition primary
# 格式化为 FAT32 格式
format fs=fat32 quick
```

然后将 SD 卡插入 Linux 服务器，使用 `fdisk` 命令制作 SD 卡启动盘

```bash
# 使用 fdisk 命令修改 SD 卡分区
sudo fdisk /dev/sda
# 删除当前分区，删除后 SD 卡应该没有任何分区
d
# 创建新主分区，分区号为 1 ，起始扇区为 20480 ，分区大小为 500M
n
p
1
20480
+500M
# 查看新创建的分区
p
# 创建新主分区，分区号为 2 ，起始扇区为 1044480
n
p
2
1044480
<enter>
# 查看新创建的分区
p
# 将分区表写入磁盘并退出
w
```

创建好分区之后，需要对分区进行格式化操作

```bash
# 将分区 sdb1 格式化为 vfat
sudo mkfs -t vfat /dev/sdb1
# 将分区 sdb2 格式化为 ext4
sudo mkfs -t ext4 /dev/sdb2
```

最终 SD 卡内部分区应该如下图所示

![[linux_sd_startup.png]]

将 U-Boot 目录中的 `u-boot.bin` 使用 `imxdownload` 工具烧录到 SD 卡的第一个分区

```bash
# 解压
tar -xvf U-Boot.tar.gz
# 进入目录
cd U-Boot
# 烧录 U-Boot 
./imxdownload u-boot.bin /dev/sdx
```

将 kernel 目录中的镜像和设备树拷贝到 SD 卡的第二个分区

```bash
# 解压
tar -xvf kernel.tar.gz
# 创建挂载文件夹
sudo mkdir tempdir
# 挂载 SD 卡第二个分区
sudo mount /dev/sdx1 tempdir/
# 拷贝镜像和设备树
sudo cp kernel/* tempdir/
# 卸载 SD 卡
sudo umount tempdir
```

将 rootfs 目录中的根文件系统拷贝到 SD 卡的第三个分区

```bash
# 解压
tar -xvf rootfs.tar.gz
# 创建挂载文件夹
sudo mkdir tempdir
# 挂载 SD 卡第三个分区
sudo mount /dev/sdx2 tempdir/
# 拷贝根文件系统
sudo cp -r rootfs/* tempdir/
# 卸载 SD 卡
sudo umount tempdir
```

## 3、从 SD 卡启动开发板

将启动方式拨码开关设置为从SD卡启动，插入SD卡然后打开电源，观察终端输出，一切正常会自动启动桌面程序