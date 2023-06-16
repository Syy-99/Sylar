/**
 * @file servlet.h
 * @brief Servlet封装
 */
#ifndef __SYLAR_HTTP_SERVLET_H__
#define __SYLAR_HTTP_SERVLET_H__

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include "http.h"
#include "http_session.h"

#include "../mutex.h"


namespace sylar {
namespace http {

/**
 * @brief Servlet封装
 */
class Servlet {
public:
    /// 智能指针类型定义
    typedef std::shared_ptr<Servlet> ptr;
    Servlet(const std::string& name) :m_name(name) {}
    virtual ~Servlet() {}

    /**
     * @brief 处理请求
     * @param[in] request HTTP请求
     * @param[in] response HTTP响应
     * @param[in] session HTTP连接
     * @return 是否处理成功
     */
    virtual int32_t handle(sylar::http::HttpRequest::ptr request
                   , sylar::http::HttpResponse::ptr response
                   , sylar::http::HttpSession::ptr session) = 0;

    
    const std::string& getName() const { return m_name;}
protected:
    /// 名称
    std::string m_name;
};


/**
 * @brief 函数式Servlet
 */
class FunctionServlet : public Servlet {
public:
    typedef std::shared_ptr<FunctionServlet> ptr;
    typedef std::function<int32_t (sylar::http::HttpRequest::ptr request, 
                                   sylar::http::HttpResponse::ptr response, 
                                   sylar::http::HttpSession::ptr session)> callback;

    FunctionServlet(callback cb);

    virtual int32_t handle(sylar::http::HttpRequest::ptr request
                        , sylar::http::HttpResponse::ptr response
                        , sylar::http::HttpSession::ptr session) override;
private:
    callback m_cb;
};

/**
 * @brief Servlet分发器
 */
class ServletDispatch : public Servlet {
 public:
    /// 智能指针类型定义
    typedef std::shared_ptr<ServletDispatch> ptr;
    /// 读写锁类型定义
    typedef RWMutex RWMutexType;

    ServletDispatch();
    virtual int32_t handle(sylar::http::HttpRequest::ptr request
                   , sylar::http::HttpResponse::ptr response
                   , sylar::http::HttpSession::ptr session) override;



    // 添加精准匹配Servlet
    void addServlet(const std::string& uri, Servlet::ptr slt);
    void addServlet(const std::string& uri, FunctionServlet::callback cb);
    // 添加模糊匹配Servlet
    void addGlobServlet(const std::string& uri, Servlet::ptr slt);
    void addGlobServlet(const std::string& uri, FunctionServlet::callback cb);

    /// 删除
    void delServlet(const std::string& uri);
    void delGlobServlet(const std::string& uri);

    // 默认Servelt
    Servlet::ptr getDefault() const { return m_default;}
    void setDefault(Servlet::ptr v) { m_default = v;}

    /// 根据uri匹配
    Servlet::ptr getServlet(const std::string& uri);
    Servlet::ptr getGlobServlet(const std::string& uri);
    /// 优先精准匹配,其次模糊匹配,最后返回默认
    Servlet::ptr getMatchedServlet(const std::string& uri);
private:   
    /// 读写互斥量
    RWMutexType m_mutex;

    /// 精准匹配servlet MAP
    /// uri(/sylar/xxx) -> servlet, 请求不同的url有不同的servlet
    std::unordered_map<std::string, Servlet::ptr> m_datas;
    /// 模糊匹配servlet 数组
    /// uri(/sylar/*) -> servlet
    std::vector<std::pair<std::string, Servlet::ptr> > m_globs;

    /// 默认servlet，所有路径都没匹配到时使用
    Servlet::ptr m_default;
};

}
}

#endif
