
/*
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-6-10     suozhang      the first version
 *
 */
 
#include "kfifo.h"

#include <string.h>

#define min(a, b)  (((a) < (b)) ? (a) : (b))

struct KFIFO *kfifo_alloc(unsigned int size) 
{
	unsigned char *buffer;

	struct KFIFO *ret;

	ret=(struct KFIFO *) pvPortMalloc(sizeof (struct KFIFO));

	/*  
	 * round up to the next power of 2, since our 'let the indices  
	 * wrap' tachnique works only in this case.  
	 * ���size ��2�� ����Բ������ size & (size - 1)  =0
	 */

	if (size & (size - 1))
	{
			// �����Ҫ�����buffer ���� 2�� ����Բ������Ҫ�� size ��� 2�Ĵ���Բ�� �������������
			// �����������û��ʵ�֣���˲��� size ������ 2�Ĵ���Բ�� ,2019��6��10��18:55:48
			//size = roundup_pow_of_two(size);
			while(1);
	}

	//����ʹ��  TI OSAL �� �����ڴ�� API
	buffer = (unsigned char*) pvPortMalloc( size );

	if (!buffer)   //������ص�ֵΪNULL����˵�������ڴ�ʧ��
			return 0UL;

	//    ret = kfifo_init(buffer, size, lock);   

	ret->buffer=buffer;
	ret->size  =size;
	ret->in  = 0;
	ret->out = 0;

	if (!ret) //���ret��ֵΪNULL����˵�������ڴ�ʧ��
			vPortFree(buffer); //�ͷ�֮ǰ����� �ڴ�ռ�

	return ret;
	
}

unsigned int kfifo_put(struct KFIFO *fifo, unsigned char *buffer, unsigned int len)
{
	unsigned int L;

	//���λ�������ʣ������Ϊfifo->size - fifo->in + fifo->out����д��ĳ���ȡlen��ʣ�������н�С�ģ�����дԽ�磻
	len = min( len , fifo->size - fifo->in + fifo->out );

	/*  
	 * Ensure that we sample the fifo->out index -before- we  
	 * start putting bytes into the kfifo.  
	 */   

	/* first put the data starting from fifo->in to buffer end */
			/* ���Ƚ����ݴ�fifo.in ���ڵ�λ�ÿ�ʼд��д֮ǰ������Ҫ��һ��fifo->in�� buffer ĩβ�Ĵ�С �ǲ��� �� len ��*/

			/*
			 * ǰ�潲��fifo->size�Ѿ�2�Ĵ���Բ������Ҫ�Ƿ���������㣬����Ч��
			 * �ڶ�10���������ʱ�����Ƿ��֣��������������еĸ�λ�ϵ����֣������ù�����λ��ʲô��
			 * ����,kfifo->in % kfifo->size ����ת��Ϊ kfifo->in & (kfifo->size �C 1)��Ч�ʻ�����
			 * ����fifo->size - (fifo->in & (fifo->size - L)) ��λ fifo->in �� bufferĩβ��ʣ��ĳ��ȣ�
			 * Lȡlen��ʣ�೤�ȵ���Сֵ����Ϊ��Ҫ����L �ֽڵ�fifo->buffer + fifo->in��λ���ϡ�
			 */ 
	L = min(len, fifo->size - (fifo->in & (fifo->size - 1)));

	memcpy(fifo->buffer + (fifo->in & (fifo->size - 1)), buffer, L);   

	/* then put the rest (if any) at the beginning of the buffer */ 

	memcpy(fifo->buffer, buffer + L, len - L);

	/*  
	 * Ensure that we add the bytes to the kfifo -before-  
	 * we update the fifo->in index.  
	 */   

		/* 
		 * ע������ ֻ������ fifo->in +=  len��δȡģ��
		 * �����kfifo����ƾ���֮���������õ���unsigned int��������ʣ�
		 * ��in �������ӵ����ʱ�ֻᱻ��Ϊ0�������ͽ�ʡ��ÿ��in��ǰ���Ӷ�Ҫȡģ�����ܣ�
		 * �����ؽϣ������󾫣����˲��ò������
		 */

	fifo->in += len; 

	/*����ֵ ����  д�����ݵĸ��� ������ �Ϳ��Ը��ݷ���ֵ �жϻ������Ƿ�д��*/
	return len;   
}  
  
unsigned int kfifo_get(struct KFIFO *fifo, unsigned char *buffer, unsigned int len)   
{
	unsigned int L;   

	len = min(len, fifo->in - fifo->out);   

	/*  
	 * Ensure that we sample the fifo->in index -before- we  
	 * start removing bytes from the kfifo.  
	 */   

	/* first get the data from fifo->out until the end of the buffer */   
	L = min(len, fifo->size - (fifo->out & (fifo->size - 1)));   
	memcpy(buffer, fifo->buffer + (fifo->out & (fifo->size - 1)), L);   

	/* then get the rest (if any) from the beginning of the buffer */   
	memcpy(buffer + L, fifo->buffer, len - L);   

	/*  
	 * Ensure that we remove the bytes from the kfifo -before-  
	 * we update the fifo->out index.  
	 */   

		/*
		 * ע������ ֻ������ fifo->out +=  len Ҳδȡģ���㣬
		 * ͬ��unsigned int��������ʣ���out �������ӵ����ʱ�ֻᱻ��Ϊ0��
		 * ���in����������� in  < out ���������ô in �C out Ϊ�������ֽ��������
		 * in �C out ��ֵ����Ϊbuffer�����ݵĳ��ȡ�
		 */

	fifo->out += len;

	return len;  
}

unsigned int kfifo_get_data_len(struct KFIFO *fifo)   
{
	return(fifo->in - fifo->out);
}

















