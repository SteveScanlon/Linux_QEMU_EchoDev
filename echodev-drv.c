/************************************* 
* File      : echodev-drv.c 
* Description : A Driver for the Custom EchoDevice
*               we built for QEMU.
* Tutorial Series : Johannes 4GNU_Linux
* Resources : youtube.com/watch?v=x_9XvvTc1BQ&t
*             github.com/NikosMouzakits/cpcidev_pci/tree/main
*
* Author    : Steve Scanlon 
* Date      : May 10 2024.
*
* ************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/delay.h>


#define DEVNR 64
#define DEVNRNAME "echodev"

#define VENDOR_ID 0x1234
#define DEVICE_ID 0xbeef

struct echodev {
        struct pci_dev *pdev;
} mydev;


static int echo_mmap(struct file *file, struct vm_area_struct *vma)
{
    int status;

    vma->vm_pgoff = pci_resource_start(mydev.pdev, 1)  >> PAGE_SHIFT;

    status = io_remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, (vma->vm_end - vma->vm_start), vma->vm_page_prot);
    if (status > 0) // Error check.
    {
        printk("echodev-drv - Error re-mapping BAR1 virtual memory area.\n");
        return -status;
    }
    return 0;
}

static struct file_operations fops = {
        .mmap = echo_mmap,
};


static struct pci_device_id echo_ids[] = {
    {PCI_DEVICE(VENDOR_ID, DEVICE_ID)},
    { },
};

MODULE_DEVICE_TABLE(pci, echo_ids);


static int echo_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
    int status;
    void __iomem *ptr_bar0;
    void __iomem *ptr_bar1;

    mydev.pdev = pdev;

    status = register_chrdev(DEVNR, DEVNRNAME, &fops);
    if (status < 0)
    {
        printk("echodev-drv - Error allocating device number. \n");
        goto fdev;
    }

    status = pcim_enable_device(pdev);

    if (status != 0)
    {
        printk("echodev-drv - Error enabling device.\n");
        goto fdev;
    }

    ptr_bar0 = pcim_iomap(pdev, 0, pci_resource_len(pdev, 0));
    /* check to see if pointer to bar0 is null.*/
    if (!ptr_bar0)
    {
        printk("echodev-drv - Error mapping BAR0\n"); 
        status = -ENODEV;
        goto fdev;
    }

    ptr_bar1 = pcim_iomap(pdev, 1, pci_resource_len(pdev, 1)); 
    /* check to see if pointer to bar1 is null.*/
    if (!ptr_bar1)
    {
        printk("echodev-drv - Error mapping BAR1\n");
        status = -ENODEV;
        goto fdev;
    }

    printk("echodev-drv - Register ID : 0x%x\n", ioread32(ptr_bar0));
    printk("echoddev-drv - Random Value: 0x%x\n", ioread32(ptr_bar0 + 0xC));

    /* Test read and write to BAR0 */
    iowrite32(0x11223344, ptr_bar0 + 0x04);
    mdelay(1);
    printk("echodev-drv - Inverse Pattern: 0x%x\n", ioread32(ptr_bar0 + 0x04));

    /* Test read and write to BAR1 */
    iowrite32(0x44332211, ptr_bar1);
    /* Read 1 BYTE access */
    printk("echodev-drv - BAR1 Offest 0: 0x%x\n", ioread8(ptr_bar1));
    printk("echodev-drv - BAR1 Offest 0: 0x%x\n", ioread16(ptr_bar1));
    printk("echodev-drv - BAR1 Offest 0: 0x%x\n", ioread32(ptr_bar1));

    return 0;

fdev:
    unregister_chrdev(DEVNR, DEVNRNAME);
    return status;

}


static void echo_remove(struct pci_dev *pdev)
{
    printk("echodev-drv - Removing the device.\n");
    unregister_chrdev(DEVNR, DEVNRNAME);
}


static struct pci_driver echo_driver = {
        .name     = "echodev-driver",
        .probe    = echo_probe,
        .remove   = echo_remove,
        .id_table = echo_ids,
};


module_pci_driver(echo_driver);


MODULE_LICENSE("GPL");











