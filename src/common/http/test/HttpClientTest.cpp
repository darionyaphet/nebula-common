/* Copyright (c) 2019 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#include "common/http/HttpClient.h"

#include <gtest/gtest.h>
#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>

#include "common/webservice/Common.h"
#include "common/webservice/Router.h"
#include "common/webservice/WebService.h"

namespace nebula {
namespace http {

using proxygen::HTTPMethod;

class HttpClientHandler : public proxygen::RequestHandler {
public:
    HttpClientHandler() = default;

    void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override {
        if (headers->getMethod().value() == HTTPMethod::GET) {
            LOG(INFO) << "Processing HTTP Get Request";
        } else if (headers->getMethod().value() == HTTPMethod::PUT) {
            LOG(INFO) << "Processing HTTP Put Request";
        } else {
            err_ = HttpCode::E_UNSUPPORTED_METHOD;
            return;
        }
    }

    void onBody(std::unique_ptr<folly::IOBuf>) noexcept override {
    }

    void onEOM() noexcept override {
        proxygen::ResponseBuilder(downstream_)
            .status(WebServiceUtils::to(HttpStatusCode::OK),
                    WebServiceUtils::toString(HttpStatusCode::OK))
            .body("HttpClientHandler successfully")
            .sendWithEOM();
    }

    void onUpgrade(proxygen::UpgradeProtocol) noexcept override {
    }

    void requestComplete() noexcept override {
        delete this;
    }

    void onError(proxygen::ProxygenError error) noexcept override {
        LOG(ERROR) << "HttpClientHandler Error: "
                   << proxygen::getErrorString(error);
    }

private:
    HttpCode err_{HttpCode::SUCCEEDED};
};

class HttpClientTestEnv : public ::testing::Environment {
public:
    void SetUp() override {
        FLAGS_ws_ip = "127.0.0.1";
        FLAGS_ws_http_port = 0;
        FLAGS_ws_h2_port = 0;
        LOG(INFO) << "Starting web service...";
        webSvc_ = std::make_unique<WebService>();

        auto& router = webSvc_->router();
        router.get("/get_path").handler([](auto&&) {
            return new HttpClientHandler();
        });
        router.post("/post_path").handler([](auto&&) {
            return new HttpClientHandler();
        });

        auto status = webSvc_->start();
        ASSERT_TRUE(status.ok()) << status;
    }

    void TearDown() override {
        webSvc_.reset();
        VLOG(1) << "Web service stopped";
    }

private:
    std::unique_ptr<WebService> webSvc_;
};

TEST(HttpClient, get) {
    {
        auto url = folly::stringPrintf("http://%s:%d%s", FLAGS_ws_ip.c_str(),
                                       FLAGS_ws_http_port, "/get_path");
        auto result = HttpClient::get(url);
        ASSERT_TRUE(result.ok());
        ASSERT_EQ("HttpClientHandler successfully", result.value());
    }
    {
        auto url = folly::stringPrintf("http://%s:%d%s", FLAGS_ws_ip.c_str(),
                                       FLAGS_ws_http_port, "/not_exist");
        auto result = HttpClient::get(url);
        ASSERT_TRUE(result.ok());
        ASSERT_TRUE(result.value().empty());
    }
}

TEST(HttpClient, post) {
    {
        auto url = folly::stringPrintf("http://%s:%d%s", FLAGS_ws_ip.c_str(),
                                       FLAGS_ws_http_port, "/post_path");
        std::string message = "Hello World!";
        auto result = HttpClient::post(url, message);
        ASSERT_TRUE(result.ok());
        ASSERT_EQ("HttpClientHandler successfully", result.value());
    }
    {
        auto url = folly::stringPrintf("http://%s:%d%s", FLAGS_ws_ip.c_str(),
                                       FLAGS_ws_http_port, "/not_exist");
        std::string message = "Hello World!";
        auto result = HttpClient::post(url, message);
        ASSERT_TRUE(result.ok());
        ASSERT_TRUE(result.value().empty());
    }
}

}   // namespace http
}   // namespace nebula

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    folly::init(&argc, &argv, true);
    google::SetStderrLogging(google::INFO);

    ::testing::AddGlobalTestEnvironment(new nebula::http::HttpClientTestEnv());
    return RUN_ALL_TESTS();
}
