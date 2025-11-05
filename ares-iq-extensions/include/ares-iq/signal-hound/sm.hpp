//
// Created by tschmitz on 11/5/25.
//

#ifndef VERSION_SM_HPP
#define VERSION_SM_HPP

struct SMConfigs {

};

class SM {
public:
    explicit SM(const SMConfigs &configs);
    ~SM();

private:
    int fd;
};

#endif //VERSION_SM_HPP