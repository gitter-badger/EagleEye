#include <nodes/Node.h>

namespace EagleLib
{
	namespace IO
	{
		class CV_EXPORTS VideoLoader : public Node
		{
		public:
			//VideoLoader();
			VideoLoader(const std::string& file = "");
			~VideoLoader();
			void loadFile();

			virtual cv::cuda::GpuMat doProcess(cv::cuda::GpuMat& img);
			//bool EOF_reached;
		};
	}
}
