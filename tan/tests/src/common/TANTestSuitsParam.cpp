#include "TANTestSuitsParam.h"
#include "samples/src/GPUUtilities/GpuUtilities.h"
#include "assert.h"
TANTestSuitsParam::TANTestSuitsParam(eTestMode mode)
{
	TANCreateContext(TAN_FULL_VERSION, &m_pContext);
	this->m_eTestMode = mode;
	if (mode == eGPU)
	{
		cl_device_id id;
		cl_context context;
		getDeviceAndContext(0, &context, &id);
		cl_command_queue queue = createQueue(context, id);
		m_pContext->InitOpenCL(queue, queue);
	}
	
}

#ifndef RTQ_ENABLED
TANTestSuitsParam::TANTestSuitsParam(eTestMode mode, eCommandQueueType _generalqueuetype, eCommandQueueType _convolutionqueuetype, int outputflag)
{
	m_eConvolutuionQueueType = _convolutionqueuetype;
	m_eGeneralQueueType = _generalqueuetype;
	m_iOutputFlag = outputflag;
	TANCreateContext(TAN_FULL_VERSION, &m_pContext);
	if (mode == eGPU)
	{
		cl_device_id id;
		cl_context context;
		getDeviceAndContext(0, &context, &id);
		int general_queue_flag = 0;
		int convolution_queue_flag = 0;
		cl_command_queue general_queue;
		cl_command_queue convolution_queue;
		switch (_generalqueuetype)
		{
			case eNormalQueue:
			{
				general_queue_flag = QUEUE_NORMAL_QUEUE;
				general_queue = createQueue(context, id, general_queue_flag);
				break;
			}
#ifdef RTQ_ENABLED
			case eMediumPriorityQueue:
			{
				general_queue_flag = QUEUE_MEDIUM_PRIORITY;
				general_queue = createQueue(context, id, general_queue_flag);
				break;
			}
			case eRealTimeQueue:
			{
				general_queue_flag = QUEUE_REAL_TIME_COMPUTE_UNITS;
				general_queue = createQueue(context, id, general_queue_flag, _generalCUcount);
				break;
			}
#endif // RTQ_ENABLED
			default:
				general_queue_flag = QUEUE_NORMAL_QUEUE;
				general_queue = createQueue(context, id, general_queue_flag);
				break;
		}
		switch (_convolutionqueuetype)
		{
		case eNormalQueue:
		{
			convolution_queue_flag = QUEUE_NORMAL_QUEUE;
			convolution_queue = createQueue(context, id, convolution_queue_flag);
			break;
		}
#ifdef RTQ_ENABLED
		case eMediumPriorityQueue:
		{
			convolution_queue_flag = QUEUE_MEDIUM_PRIORITY;
			convolution_queue = createQueue(context, id, convolution_queue_flag);
			break;
		}
		case eRealTimeQueue:
		{
			convolution_queue_flag = QUEUE_REAL_TIME_COMPUTE_UNITS;
			convolution_queue = createQueue(context, id, convolution_queue_flag, _convolutionCUcount);
			break;
		}
#endif // RTQ_ENABLED
			default:
			general_queue_flag = QUEUE_NORMAL_QUEUE;
			convolution_queue = createQueue(context, id, general_queue_flag);
			break;
		}
		
		m_pContext->InitOpenCL(general_queue, convolution_queue);
	}
	this->m_eTestMode = mode;
}
#endif

void TANTestSuitsParam::PrintTestSuitesParam()
{
	std::string testmode = "";
	m_eTestMode == eCPU ? testmode = "CPU" : testmode = "GPU";
	printf("Test mode: %s\n", testmode.c_str());
	switch (m_eGeneralQueueType)
	{
	case eNormalQueue:
	{
		printf("General Queue Type: Normal Queue, ");
		break;
	}
#ifdef RTQ_ENABLED
	case eMediumPriorityQueue:
	{
		printf("General Queue Type: Medium Priority Queue, ");
		break;
	}
	case eRealTimeQueue:
	{
		printf("General Queue Type: Real Time Queue, CU Count: %d, ", m_iGeneralQueueCUCount);
		break;
	}
#endif
	default:
		printf("General Queue Type: Unknown. ");
		break;
	}
	switch (m_eConvolutuionQueueType)
	{
	case eNormalQueue:
	{
		printf("Convolution Queue Type: Normal Queue. ");
		break;
	}
#ifdef RTQ_ENABLED
	case eMediumPriorityQueue:
	{
		printf("Convolution Queue Type: Medium Priority Queue. ");
		break;
	}
	case eRealTimeQueue:
	{
		printf("Convolution Queue Type: Real Time Queue, CU Count: %d. ", m_iGeneralQueueCUCount);
		break;
	}
#endif // DEBUG

	default:
		printf("Convolution Queue Type: Unknown. ");
		break;
	}
	printf("\n");
}

std::wstring TANTestSuitsParam::getTestSuiteParamWStr()
{
	std::wstring testmode = L"";
	m_eTestMode == eCPU ? testmode += L"CPU" : testmode += L"GPU";
	switch (m_eGeneralQueueType)
	{
	case eNormalQueue:
	{ 
		testmode += L"_GENQUE-NMQ";
		break;
	}
#ifdef RTQ_ENABLED
	case eMediumPriorityQueue:
	{
		testmode += L"_GENQUE-MPQ";
		break;
	}
	case eRealTimeQueue:
	{
		testmode += L"_GENQUE_RTQ";
		break;
	}
#endif // RTQ_ENABLED
	default:
		break;
	}
	switch (m_eConvolutuionQueueType)
	{
	case eNormalQueue:
	{
		testmode += L"_CONQUE-NMQ";
		break;
	}
#ifdef RTQ_ENABLED
	case eMediumPriorityQueue:
	{
		testmode += L"_CONQUE-MPQ";
		break;
	}
	case eRealTimeQueue:
	{
		testmode += L"_CONQUE-RTQ";
		break;
	}
#endif // RTQ_ENABLED

	default:
		break;
	}
	return testmode;
}
