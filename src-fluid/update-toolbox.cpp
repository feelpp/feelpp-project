#include <feel/feelcore/environment.hpp>
#include <feel/feelmodels/fluid/fluidmechanics.hpp>

int main(int argc, char** argv)
{
    using namespace Feel;
    po::options_description myoptions("my options");
    myoptions.add( toolboxes_options("fluid") );
    myoptions.add_options()
        ( "formulation", po::value<std::string>()->default_value("breakconvection"), "keep or break convection term")
        ;

    Environment env( _argc=argc, _argv=argv,
                     _desc=myoptions,
                     _about=about(_name="update_toolbox",
                                  _author="Feel++ Consortium",
                                  _email="feelpp-devel@feelpp.org"));

    typedef FeelModels::FluidMechanics< Simplex<2,1>,
                                  Lagrange<2, Vectorial,Continuous,PointSetFekete> > model_type;
    std::shared_ptr<model_type> fluid( new model_type("fluid") );
    // init the toolbox
    fluid->init();
    fluid->printAndSaveInfo();

    if( soption("formulation")=="breakconvection" ) {
        auto lambda = [&fluid](FeelModels::ModelAlgebraic::DataUpdateResidual & data) {
                        // build each time, not just for the constant part
                        bool buildCstPart = data.buildCstPart();
                        if( buildCstPart )
                            return;
                        // retrieve matrix and vector already assemble
                        vector_ptrtype& F = data.rhs();
                        // retrieve space and create a test function
                        auto Vh = fluid->functionSpaceVelocity();
                        auto Wh = fluid->functionSpacePressure();
                        auto v = Vh->element();
                        auto q = Wh->element();
                        // retrieve current approximate
                        auto U = data.currentSolution();
                        auto u = fluid->functionSpaceVelocity()->element( U, 0 );
                        auto p = fluid->functionSpacePressure()->element( U, 1 );

                        // create a linear form from the vector on the function space
                        auto f = form1( _test=Vh, _vector=F );
                        // add the term to the linear form
                        f += integrate( _range=elements(support(Vh)), _expr=2*mu*inner( sym(gradv(u)), sym(grad(v)) ) );
                        f += integrate( _range=elements(support(Vh)), _expr=-idv(p)*div(v) );
                        f += integrate( _range=elements(support(Vh)), _expr=-inner(idv(u), idv(u))*div(v) );
                    };
        auto mu = [&fluid](FeelModels::ModelAlgebraic::DataUpdateJacobian & data) {
                        // build each time, not just for the constant part
                        bool buildCstPart = data.buildCstPart();
                        if( buildCstPart )
                            return;
                        // retrieve matrix and vector already assemble
                        sparse_matrix_ptrtype& A = data.matrix();
                        // retrieve space and create a test function
                        auto Vh = fluid->functionSpaceVelocity();
                        auto Wh = fluid->functionSpacePressure();
                        auto v = Vh->element();
                        auto q = Wh->element();
                        // retrieve current approximate
                        auto U = data.currentSolution();
                        auto u = fluid->functionSpaceVelocity()->element( U, 0 );
                        auto p = fluid->functionSpacePressure()->element( U, 1 );

                        // create a bilinear form from the matrix on the function space
                        auto a = form2( _test=Vh, _trial=Vh );
                        // add the term to the bilinear form
                        a += integrate( _range=elements(support(Vh)), _expr=-2*mu*inner( sym(gradt(v)), sym(grad(v)) ) );
                        a += integrate( _range=elements(support(Vh)), _expr=-idt(q)*div(v) );
                        a += integrate( _range=elements(support(Wh)), _expr=-id(q)*divt(v) );
                        a += integrate( _range=elements(support(Vh)), _expr=-inner(idt(v), idv(u))*div(v) );
                    };
        // add the functions to the algebraic factory
        fluid->algebraicFactory()->addFunctionResidualAssembly(lambda);
        fluid->algebraicFactory()->setFunctionJacobianAssembly(mu);
    }
    else {
        auto lambda = [&fluid](FeelModels::ModelAlgebraic::DataUpdateResidual & data) {
                        // build each time, not just for the constant part
                        bool buildCstPart = data.buildCstPart();
                        if( buildCstPart )
                            return;
                        // retrieve matrix and vector already assemble
                        vector_ptrtype& F = data.rhs();
                        // retrieve space and create a test function
                        auto Vh = fluid->functionSpaceVelocity();
                        auto v = Vh->element();
                        // retrieve current approximate
                        auto U = data.currentSolution();
                        auto u = fluid->functionSpaceVelocity()->element( U, 0 );

                        // create a linear form from the vector on the function space
                        auto f = form1( _test=Vh, _vector=F);
                        // add the term to the linear form
                        f += integrate( _range=markedfaces(support(Vh), "Gamma"), _expr=0.5*inner(idv(u), idv(u))*div(v) );
                    };
        auto mu = [&fluid](FeelModels::ModelAlgebraic::DataUpdateJacobian & data) {
                        // build each time, not just for the constant part
                        bool buildCstPart = data.buildCstPart();
                        if( buildCstPart )
                            return;
                        // retrieve matrix and vector already assemble
                        sparse_matrix_ptrtype& A = data.matrix();
                        // retrieve space and create a test function
                        auto Vh = fluid->functionSpaceVelocity();
                        auto v = Vh->element();
                        // retrieve current approximate
                        auto U = data.currentSolution();
                        auto u = fluid->functionSpaceVelocity()->element( U, 0 );

                        // create a bilinear form from the matrix on the function space
                        auto a = form2( _test=Vh, _trial=Vh, _matrix=A);
                        // add the term to the bilinear form
                        a += integrate( _range=markedfaces(support(Vh), "Gamma"), _expr=-inner(idt(v), idv(u))*div(v) );
                    }; 

        // add the functions to the algebraic factory
        fluid->algebraicFactory()->addFunctionResidualAssembly(lambda);
        fluid->algebraicFactory()->addFunctionJacobianAssembly(mu);
    }

    // solve the problem
    fluid->solve();
    fluid->exportResults();

    return 0;
}
