#include <torch/csrc/distributed/rpc/python_rpc_handler.h>

namespace torch {
namespace distributed {
namespace rpc {
namespace {
py::object runUDFFunction_ = py::none();
py::object loadResultFunction_ = py::none();
} // anonymous namespace

namespace PythonRpcHandler {
void init() {
  AutoGIL ag;
  py::object module =
      py::module::import("torch.distributed.internal_rpc_utils");
  TORCH_CHECK(!module.is(py::none()), "module is None.");
  if (runUDFFunction_.is(py::none())) {
    runUDFFunction_ = module.attr("run_python_udf_internal");
  }
  if (loadResultFunction_.is(py::none())) {
    loadResultFunction_ = module.attr("load_python_udf_result_internal");
  }
}

std::vector<char> generatePythonUDFResult(const Message& request) {
  AutoGIL ag;
  auto pargs = py::bytes(request.payload().data(), request.payload().size());
  TORCH_CHECK(!runUDFFunction_.is(py::none()), "runUDFFunction_ is None.");
  py::bytes pres = runUDFFunction_(pargs);
  const auto& presStr = static_cast<std::string>(pres);
  std::vector<char> payload(presStr.begin(), presStr.end());
  return payload;
}

py::object loadPythonUDFResult(const Message& message) {
  AutoGIL ag;
  auto pargs = py::bytes(message.payload().data(), message.payload().size());
  TORCH_CHECK(
      !loadResultFunction_.is(py::none()), "loadResultFunction_ is None.");
  return loadResultFunction_(pargs);
}
} // namespace PythonRpcHandler

} // namespace rpc
} // namespace distributed
} // namespace torch
