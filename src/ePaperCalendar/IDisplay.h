#pragma once
// Local

// Internal Library
#include <ePaperDriver.h>

// Extenral Library

class IDisplay {
    public:
        virtual void show();
        virtual ~IDisplay() = default;
    protected:
        IDisplay(EpaperDriver& display) : m_display(display) {}  // pass in constructor
        
        EpaperDriver& m_display;


};