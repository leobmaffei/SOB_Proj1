#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/crypto.h>
#include <linux/scatterlist.h>
#include <crypto/hash.h>
#include <crypto/sha.h>
#include <crypto/skcipher.h>
#include <crypto/aes.h>
#include <linux/random.h>
#include <linux/err.h>

#define DEVICE_NAME "PROJ1"
#define CLASS_NAME  "proj1"
#define BUFFER_LENGTH 256 ///< The buffer length (crude but fine)
#define DATA_SIZE     16
#define IV_SIZE       256
#define KEY_SIZE      32

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Grupo SOb");
MODULE_DESCRIPTION("Projeto 1 - Sistemas Operacionais B");
MODULE_VERSION("0.1");

static char *key = "ValorTeste"; //Chave simétrica que será usada para cifrar e decifrar

static int majorNumber;
static char message[256] = {0};
static short size_of_message;
static int numberOpens = 0;
static struct class* proj1Class = NULL;
static struct device* proj1Device = NULL;
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static int test_hash(const unsigned char *data, unsigned int datalen, unsigned char *digest);

static struct file_operations fops =
{
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

module_param(key, charp, 0000);
MODULE_PARM_DESC(key, "Chave simétrica que será usada para cifrar e decifrar");


/* Initialize and trigger cipher operation */
static int encripta_decripta(unsigned char *key, unsigned char *input, unsigned char *output, unsigned char operacao)
{
    struct scatterlist sg_input;
    struct scatterlist sg_output;
    struct crypto_skcipher *skcipher = NULL;
    struct skcipher_request *req = NULL;
    int ret = -EFAULT;

    unsigned char ivdata[IV_SIZE];

    /* inicializa skcipher */
    skcipher = crypto_alloc_skcipher("cbc-aes-aesni", 0, 0);
    if (IS_ERR(skcipher)) {
        pr_info("could not allocate skcipher handle\n");
        return PTR_ERR(skcipher);
    }
    /* inicializa request handler */
    req = skcipher_request_alloc(skcipher, GFP_KERNEL);
    if (!req) {
        pr_info("could not allocate skcipher request\n");
        ret = -ENOMEM;
        goto out;
    }
    /* inicializa a key */
    if (crypto_skcipher_setkey(skcipher, key, KEY_SIZE)) {
        pr_info("key could not be set\n");
        ret = -EAGAIN;
        goto out;
    }

    /* inicializa scatterlists */
    sg_init_one(&sg_input, input, DATA_SIZE);
    sg_init_one(&sg_output, output, DATA_SIZE);

    
    /* define o ivdata. ivdata precisa ser constante ao encriptar e decriptar */
    memset(ivdata, '0', IV_SIZE);
    /* define os dados do request handler */
    skcipher_request_set_crypt(req, &sg_input, &sg_output, DATA_SIZE, ivdata);

    switch(operacao) {
    case 'c':
        ret = crypto_skcipher_encrypt(req);
        if (ret)
            goto out;
        pr_info("Encryption triggered successfully\n");
        break;
    case 'd':
        ret = crypto_skcipher_decrypt(req);
        if (ret)
            goto out;
        pr_info("Decryption triggered successfully\n");
        break;
    default:
        goto out;
    }    
  
out:
    if (skcipher)
        crypto_free_skcipher(skcipher);
    if (req)
        skcipher_request_free(req);
    return ret;
}

static int __init proj1_init(void) {
    printk(KERN_INFO "Proj1: Initializing the Proj1 \n");
    printk(KERN_INFO "Valor de  KEY:%s\n", key);

// Try to dynamically allocate a major number for the device -- more difficult but worth it
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber<0) {
        printk(KERN_ALERT "Proj1 failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "Proj1: registered correctly with major number %d\n", majorNumber);
// Register the device class
    proj1Class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(proj1Class)) { // Check for error and clean up if there is
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(proj1Class); // Correct way to return an error on a pointer
    }
    printk(KERN_INFO "Proj1: device class registered correctly\n");
// Register the device driver
    proj1Device = device_create(proj1Class, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(proj1Device)) { // Clean up if there is an error
        class_destroy(proj1Class); // Repeated code but the alternative is goto statements
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(proj1Device);
    }
    printk(KERN_INFO "Proj1: device class created correctly\n"); // Made it! device was initialized
    return 0;
}


/** @brief The LKM cleanup function
* Similar to the initialization function, it is static. The __exit macro notifies that if this
* code is used for a built-in driver (not a LKM) that this function is not required.
*/
static void __exit proj1_exit(void) {
    device_destroy(proj1Class, MKDEV(majorNumber, 0)); // remove the device
    class_unregister(proj1Class); // unregister the device class
    class_destroy(proj1Class); // remove the device class
    unregister_chrdev(majorNumber, DEVICE_NAME); // unregister the major number
    printk(KERN_INFO "Proj1: Goodbye from the Proj1 !\n");
}


/** @brief The device open function that is called each time the device is opened
* This will only increment the numberOpens counter in this case.
* @param inodep A pointer to an inode object (defined in linux/fs.h)
* @param filep A pointer to a file object (defined in linux/fs.h)
*/
static int dev_open(struct inode *inodep, struct file *filep) {
    numberOpens++;
    printk(KERN_INFO "Proj1: Device has been opened %d time(s)\n", numberOpens);
    return 0;
}


/** @brief This function is called whenever device is being read from user space i.e. data is
* being sent from the device to the user. In this case is uses the copy_to_user() function to
* send the buffer string to the user and captures any errors.
* @param filep A pointer to a file object (defined in linux/fs.h)
* @param buffer The pointer to the buffer to which this function writes the data
* @param len The length of the b
* @param offset The offset if required
*/
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    int error_count = 0;
    // copy_to_user has the format ( * to, *from, size) and returns 0 on success
    error_count = copy_to_user(buffer, message, size_of_message);
    if (error_count==0) { // if true then have success
        printk(KERN_INFO "Proj1: Sent %d characters to the user\n", size_of_message);
        printk(KERN_INFO "Proj1: String content: %s \n", message);
        return (size_of_message=0); // clear the position to the start and return 0
    }
    else {
        printk(KERN_INFO "Proj1: Failed to send %d characters to the user\n", error_count);
        return -EFAULT; // Failed -- return a bad address message (i.e. -14)
    }
}


/** @brief This function is called whenever the device is being written to from user space i.e.
* data is sent to the device from the user. The data is copied to the message[] array in this
* LKM using the sprintf() function along with the length of the string.
* @param filep A pointer to a file object
* @param buffer The buffer to that contains the string to write to the device
* @param len The length of the array of data that is being passed in the const char buffer
* @param offset The offset if required
*/ 
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {

    char opcao[BUFFER_LENGTH];
    int ret=0;
    unsigned char *digest = vmalloc(16); //retorno do hash
    unsigned char *input = vmalloc(16);
    unsigned char *output = vmalloc(16);

    strcpy( opcao, buffer );
    buffer = buffer + 1; 

    switch(opcao[0]){
       case 'c':
          sprintf(input, "%s", buffer);

          printk(KERN_INFO "-------------------------------------------------------------------------");
          encripta_decripta(key, input, output, 'c');
          printk(KERN_INFO "Proj1: Dado CIFRADO: %.16s", output);
          strcpy( message, output );
          size_of_message = strlen(message); 


          //sprintf(message, "%s(Cifrar)", buffer);   // appending received string with its length
          //size_of_message = strlen(message);                 // store the length of the stored message
          //printk(KERN_INFO "Proj1: Received %zu characters from the user\n", len);
          //printk(KERN_INFO "Proj1: Data received: %s \n", buffer);


          return len;
          break;
       case 'd':
          sprintf(input, "%s", buffer);
	printk(KERN_INFO "IMPUT: %.16s", buffer);
          printk(KERN_INFO "-------------------------------------------------------------------------");
          encripta_decripta(key, input, output, 'd');
          printk(KERN_INFO "DECIFRADO: %.16s", output);
          strcpy( message, output );
          size_of_message = strlen(message); 


          //sprintf(message, "%s(Decifrar)", buffer);   // appending received string with its length
          //size_of_message = strlen(message);                 // store the length of the stored message
          //printk(KERN_INFO "Proj1: Received %zu characters from the user\n", len);
          //printk(KERN_INFO "Proj1: Data received: %s \n", buffer);


          return len;
          break;
       case 'h':
          sprintf(message, "%s", buffer);   // appending received string with its length
          size_of_message = strlen(message);                 // store the length of the stored message
 
          ret = test_hash(message, size_of_message, digest);
 
          printk(KERN_INFO "-------------------------------------------------------------------------");
          printk(KERN_INFO "Proj1: HASH = %s \n", digest);
          strcpy( message, digest );
          size_of_message = strlen(message); 


          //printk(KERN_INFO "Proj1: Received %zu characters from the user\n", len);
          //printk(KERN_INFO "Proj1: Data received: %s \n", message);
          //printk(KERN_INFO "Proj1: HASH = %s \n", digest);

         
          return len;
          break;
       default:
          break;
    }

    return len;
}


/** @brief The device release function that is called whenever the device is closed/released by
* the userspace program
* @param inodep A pointer to an inode object (defined in linux/fs.h)
* @param filep A pointer to a file object (defined in linux/fs.h)
*/
static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "Proj1: Device successfully closed\n");
    return 0;
}


/** @brief HASH
 */
struct sdesc {
   struct shash_desc shash;
   char ctx[];
};

static struct sdesc *init_sdesc(struct crypto_shash *alg)
{
   struct sdesc *sdesc;
   int size;

   size = sizeof(struct shash_desc) + crypto_shash_descsize(alg);
   sdesc = kmalloc(size, GFP_KERNEL);
   if (!sdesc)
      return ERR_PTR(-ENOMEM);
   sdesc->shash.tfm = alg;
   sdesc->shash.flags = 0x0;
   return sdesc;
}

static int calc_hash(struct crypto_shash *alg, const unsigned char *data, unsigned int datalen, unsigned char *digest)
{
   struct sdesc *sdesc;
   int ret;

   sdesc = init_sdesc(alg);
   if (IS_ERR(sdesc)) {
      pr_info("can't alloc sdesc\n");
      return PTR_ERR(sdesc);
   }

   ret = crypto_shash_digest(&sdesc->shash, data, datalen, digest);
   kfree(sdesc);
   return ret;
}


static int test_hash(const unsigned char *data, unsigned int datalen, unsigned char *digest)
{
   struct crypto_shash *alg;
   char *hash_alg_name = "sha1";//tem que ser o sha1 apenas  ver em cat /proc/crypto
   int ret;

   alg = crypto_alloc_shash(hash_alg_name, 0, CRYPTO_ALG_ASYNC);
   if (IS_ERR(alg)) {
      pr_info("can't alloc alg %s\n", hash_alg_name);
      return PTR_ERR(alg);
   }
   ret = calc_hash(alg, data, datalen, digest);
   crypto_free_shash(alg);
   return ret;
}



/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
* identify the initialization function at insertion time and the cleanup function (as
* listed above)
*/
module_init(proj1_init);
module_exit(proj1_exit);
