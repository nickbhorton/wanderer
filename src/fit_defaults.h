#pragma once

#include "fit_developer_field_description.hpp"
#include "fit_developer_field_description_listener.hpp"

class DefaultDeveloperFieldDescriptionListener
    : public fit::DeveloperFieldDescriptionListener
{
    void OnDeveloperFieldDescription(const fit::DeveloperFieldDescription& desc
    ) override;
};
