#include <osgpcl/common.h>


#include <osgpcl/point_cloud_reader.h>
#include <osgpcl/outofcore_octree_reader.h>
#include <osgpcl/surfel.h>
#include <osgpcl/utility_point_types.h>

#include <osgGA/FirstPersonManipulator>

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>

#include <iostream>
#include <pcl/console/print.h>

#include <boost/program_options.hpp>
namespace po=boost::program_options;


class CameraFPManipulator : public  osgGA::FirstPersonManipulator {
  public:

  CameraFPManipulator () :  osgGA::FirstPersonManipulator(){
    setWheelMovement(0.1, false);
  }

  private:
 // virtual bool handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );


};

#include <osgDB/WriteFile>

int main(int argc, char** argv){
  po::options_description desc("./cloud_viewer [options] input ... ");

  std::vector<std::string> infiles;

  desc.add_options()
    ("help", "produce help message")
    ("input,i",po::value<std::vector<std::string> >(&infiles)->multitoken()->required(), "input point cloud ")
    ("sampling_rate,s", po::value<float>(), "randomly subsample the input cloud")
    ("depth,d", po::value<int>(), "octree depth to visualize")
    ("point_size,p", po::value<int>(), "point quad size in pixels")
    ("color,C",  "Render point cloud using RGB field")
    ("intensity,I",  "Render point cloud using intensity field")
    ("label,L",  "Render point cloud using label field")
    ("range,R", po::value<std::string>(), "Render point cloud using field range.  specify the field")
    ("surfel,S", "Render surfel point cloud ")
    ("flight,F", "use flight camera manipulator ")
    ("verbose,V", "verbose debug output ")
     ;

  po::positional_options_description p;
  p.add("input",-1);

  po::variables_map vm;
 try{
  po::store(po::command_line_parser(argc, argv).
  options(desc).positional(p).run(), vm);
  po::notify(vm);
 }
 catch( const std::exception& e)
 {
     std::cerr << e.what() << std::endl;
     std::cout << desc << std::endl;
     return 1;
 }

 if (vm.count("verbose") )   pcl::console::setVerbosityLevel(pcl::console::L_VERBOSE);
 else pcl::console::setVerbosityLevel(pcl::console::L_INFO);
 osg::ref_ptr<osgpcl::PointCloudFactory> factory;

 if(vm.count("intensity") && vm.count("surfel")){
	 factory =  new osgpcl::SurfelFactoryI<pcl::PointXYZ, pcl::Normal, pcl::Intensity>(0.03) ;
   pcl::console::print_info("Using SurfelFactoryI for Rendering...\n");

 }
 else if (vm.count("color")){
	 factory =  new osgpcl::PointCloudRGBFactory<pcl::PointXYZ, pcl::RGB>() ;
   pcl::console::print_info("Using RGB Field for Rendering...\n");
 }
 else if (vm.count("intensity")){
	 factory =  new osgpcl::PointCloudIFactory<pcl::PointXYZ, pcl::Intensity>() ;
   pcl::console::print_info("Using Intensity Field for Rendering...\n");
 }
 else if (vm.count("label")){
	 factory =   new osgpcl::PointCloudLabelFactory<pcl::PointXYZ, pcl::Label>() ;
   pcl::console::print_info("Using Label Field for Rendering...\n");
 }
 else if (vm.count("range")){
	 std::string field = vm["range"].as<std::string>();
	 factory =   new osgpcl::PointCloudCRangeFactory<pcl::PointXYZ, pcl::PointXYZ>(field) ;
   pcl::console::print_info("Using %s for Rendering...\n", field.c_str());
 }
 else if (vm.count("surfel")){
	 factory =  new osgpcl::SurfelFactoryFF<pcl::PointXYZ, pcl::Normal, osgpcl::RadiusPointT>() ;
   pcl::console::print_info("Using SurfelFactoryFF   for Rendering...\n");
 }

 osg::ref_ptr< osgpcl::CloudReaderOptions>  options = new osgpcl::CloudReaderOptions;

 if (vm.count("depth")){
	 osgpcl::OutofCoreOctreeReader::OutOfCoreOptions * opts = new osgpcl::OutofCoreOctreeReader::OutOfCoreOptions;
	 opts->setDepth(vm["depth"].as<int>(), vm["depth"].as<int>());
	 options = opts;
   pcl::console::print_info("Loading octree at depth %d\n", vm["depth"].as<int>());
 }

 if( vm.count("sampling_rate")){
   options->setSamplingRate(vm["sampling_rate"].as<float>() );
 }


 if (vm.count("point_size")){
	 factory->setPointSize(vm["point_size"].as<int>());
 }
 options->setFactory(factory);

osgViewer::Viewer viewer;
viewer.setUpViewInWindow(0,0,500,500,0);

osgDB::Registry::instance()->addReaderWriter(new osgpcl::PointCloudReader);
osgDB::Registry::instance()->addReaderWriter(new osgpcl::OutofCoreOctreeReader);

osg::Group * group = new osg::Group;
for(int i=0; i<infiles.size(); i++) {
  pcl::console::print_info("Loading :  %s \n", infiles[i].c_str());
  group->addChild(osgDB::readNodeFile(infiles[i], options));
}

viewer.setSceneData(group);
viewer.getCamera()->setClearColor( osg::Vec4(0,0,0,1));

if (vm.count("flight")){
  viewer.setCameraManipulator(new  CameraFPManipulator);
}

std::cout<< "Loading finished\n";

return viewer.run();

return 0;
}
