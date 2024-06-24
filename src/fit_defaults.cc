#include "fit_defaults.h"
#include <iostream>

void DefaultDeveloperFieldDescriptionListener::OnDeveloperFieldDescription(
    const fit::DeveloperFieldDescription& desc
)
{
    std::cout << "New Developer Field Description\n";
    std::cout << "   App Version: %d\n", desc.GetApplicationVersion();
    std::cout << "   Field Number: %d\n", desc.GetFieldDefinitionNumber();
}
